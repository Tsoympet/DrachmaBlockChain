# Production Monitoring Guide

## Overview

This guide explains how to set up production-grade monitoring for PARTHENON CHAIN mainnet nodes using Prometheus, Grafana, and alerting.

## Architecture

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Seed Node  │────▶│ Prometheus  │────▶│   Grafana   │
└─────────────┘     └─────────────┘     └─────────────┘
                           │
┌─────────────┐            │            ┌──────────────┐
│  Full Node  │────────────┘            │ Alertmanager │
└─────────────┘                         └──────────────┘
                                               │
                                               ▼
                                        ┌──────────────┐
                                        │   Email/SMS  │
                                        └──────────────┘
```

## Components

### Prometheus
- Time-series database for metrics
- Scrapes metrics from nodes every 15 seconds
- Evaluates alert rules
- Stores 30 days of data (configurable)

### Grafana
- Visualization and dashboards
- Real-time monitoring
- Historical analysis
- Customizable alerts

### Alertmanager (Optional)
- Aggregates and routes alerts
- Sends notifications (email, Slack, PagerDuty, etc.)
- Deduplicates and silences alerts

### Node Exporter (Optional)
- System metrics (CPU, memory, disk, network)
- Runs on each monitored server
- Provides OS-level insights

## Quick Start

### 1. Install Prometheus and Grafana

```bash
# Using Docker Compose (recommended)
cat > docker-compose.monitoring.yml <<EOF
version: '3.9'

services:
  prometheus:
    image: prom/prometheus:latest
    container_name: prometheus
    restart: unless-stopped
    volumes:
      - ./mainnet/monitoring/prometheus.yml:/etc/prometheus/prometheus.yml:ro
      - ./mainnet/monitoring/alerts.yml:/etc/prometheus/alerts.yml:ro
      - prometheus-data:/prometheus
    command:
      - '--config.file=/etc/prometheus/prometheus.yml'
      - '--storage.tsdb.path=/prometheus'
      - '--storage.tsdb.retention.time=30d'
      - '--web.console.libraries=/usr/share/prometheus/console_libraries'
      - '--web.console.templates=/usr/share/prometheus/consoles'
    ports:
      - "9090:9090"

  grafana:
    image: grafana/grafana:latest
    container_name: grafana
    restart: unless-stopped
    depends_on:
      - prometheus
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=changeme
      - GF_USERS_ALLOW_SIGN_UP=false
    volumes:
      - grafana-data:/var/lib/grafana
    ports:
      - "3000:3000"

  alertmanager:
    image: prom/alertmanager:latest
    container_name: alertmanager
    restart: unless-stopped
    volumes:
      - ./mainnet/monitoring/alertmanager.yml:/etc/alertmanager/alertmanager.yml:ro
      - alertmanager-data:/alertmanager
    command:
      - '--config.file=/etc/alertmanager/alertmanager.yml'
      - '--storage.path=/alertmanager'
    ports:
      - "9093:9093"

volumes:
  prometheus-data:
  grafana-data:
  alertmanager-data:
EOF

# Start monitoring stack
docker-compose -f docker-compose.monitoring.yml up -d
```

### 2. Configure Prometheus Targets

Edit `mainnet/monitoring/prometheus.yml` to add your nodes:

```yaml
scrape_configs:
  - job_name: 'drachma-mainnet-seeds'
    static_configs:
      - targets:
          - 'your-seed1.example.com:9311'
          - 'your-seed2.example.com:9311'
          - 'your-seed3.example.com:9311'
```

### 3. Enable Metrics on Nodes

Add to your node configuration:

```bash
# In /etc/drachma/seed-node-1.conf
metricsport=9311
```

Restart the node:
```bash
sudo systemctl restart drachma-seed-node-1
```

### 4. Access Dashboards

- **Prometheus:** http://localhost:9090
- **Grafana:** http://localhost:3000 (admin/changeme)

## Grafana Dashboard Setup

### 1. Add Prometheus Data Source

1. Log in to Grafana (http://localhost:3000)
2. Go to Configuration → Data Sources
3. Click "Add data source"
4. Select "Prometheus"
5. Set URL: `http://prometheus:9090`
6. Click "Save & Test"

### 2. Import Dashboard

The project includes a pre-built dashboard at `testnet/monitoring/grafana_dashboard.json`.

1. Go to Dashboards → Import
2. Upload `testnet/monitoring/grafana_dashboard.json`
3. Select Prometheus data source
4. Click "Import"

### 3. Key Panels to Monitor

- **Block Height**: Current blockchain height
- **Peer Count**: Number of connected peers
- **Mempool Size**: Transaction backlog
- **Network Hash Rate**: Total network mining power
- **Sync Status**: Is the node synchronized?
- **Chain Tips**: Number of competing chain tips (fork detection)

## Alert Configuration

### Alertmanager Setup

Create `mainnet/monitoring/alertmanager.yml`:

```yaml
---
global:
  smtp_smarthost: 'smtp.gmail.com:587'
  smtp_from: 'alerts@drachma.org'
  smtp_auth_username: 'alerts@drachma.org'
  smtp_auth_password: 'your-app-password'

route:
  group_by: ['alertname', 'cluster']
  group_wait: 10s
  group_interval: 10s
  repeat_interval: 12h
  receiver: 'team-emails'

  routes:
    - match:
        severity: critical
      receiver: 'critical-pager'
      continue: true

receivers:
  - name: 'team-emails'
    email_configs:
      - to: 'ops-team@example.com'
        send_resolved: true

  - name: 'critical-pager'
    pagerduty_configs:
      - service_key: 'your-pagerduty-key'

inhibit_rules:
  - source_match:
      severity: 'critical'
    target_match:
      severity: 'warning'
    equal: ['alertname', 'instance']
```

### Slack Integration

Add to receivers:

```yaml
  - name: 'slack-notifications'
    slack_configs:
      - api_url: 'https://hooks.slack.com/services/YOUR/WEBHOOK/URL'
        channel: '#drachma-alerts'
        title: 'PARTHENON CHAIN Alert'
        text: '{{ range .Alerts }}{{ .Annotations.description }}{{ end }}'
```

## Node Exporter (System Metrics)

### Installation

```bash
# Download
wget https://github.com/prometheus/node_exporter/releases/download/v1.7.0/node_exporter-1.7.0.linux-amd64.tar.gz

# Extract
tar xvfz node_exporter-1.7.0.linux-amd64.tar.gz
sudo cp node_exporter-1.7.0.linux-amd64/node_exporter /usr/local/bin/

# Create systemd service
sudo cat > /etc/systemd/system/node_exporter.service <<EOF
[Unit]
Description=Node Exporter
After=network.target

[Service]
Type=simple
User=nobody
ExecStart=/usr/local/bin/node_exporter
Restart=always

[Install]
WantedBy=multi-user.target
EOF

# Start service
sudo systemctl daemon-reload
sudo systemctl start node_exporter
sudo systemctl enable node_exporter
```

Access metrics at: http://localhost:9100/metrics

## Queries and Metrics

### Useful Prometheus Queries

```promql
# Current block height
drachma_blocks

# Peer count
drachma_peers_count

# Mempool size (bytes)
drachma_mempool_bytes

# Time since last block (seconds)
time() - drachma_last_block_time

# Chain sync percentage
(drachma_blocks / drachma_estimated_height) * 100

# Network hash rate
drachma_networkhashps

# Transaction rate (per second)
rate(drachma_tx_count[5m])
```

### Custom Metrics

If you need custom metrics, add them to the node:

```cpp
// In layer1-core/metrics/prometheus.cpp
metrics.RegisterGauge("drachma_custom_metric", "Description");
metrics.SetGauge("drachma_custom_metric", value);
```

## Production Best Practices

### 1. Security

- **Firewall**: Block metrics ports (9090, 9100, 9311) from external access
- **Authentication**: Enable Grafana authentication
- **HTTPS**: Use reverse proxy (nginx) with SSL
- **Secrets**: Don't commit API keys or passwords

### 2. Reliability

- **Redundancy**: Run Prometheus in HA mode
- **Backups**: Backup Grafana dashboards and datasources
- **Persistence**: Use volumes for data retention
- **Updates**: Keep monitoring stack updated

### 3. Performance

- **Retention**: Adjust based on needs (default: 30 days)
- **Scrape interval**: Balance between granularity and load
- **Resource allocation**: Give Prometheus adequate CPU/RAM

### 4. Alerting

- **Alert fatigue**: Tune thresholds to avoid noise
- **Runbooks**: Document response procedures
- **On-call**: Rotate critical alerts among team
- **Testing**: Regularly test alert delivery

## Nginx Reverse Proxy (Optional)

For production, expose monitoring via HTTPS:

```nginx
server {
    listen 443 ssl http2;
    server_name monitoring.drachma.org;

    ssl_certificate /etc/letsencrypt/live/monitoring.drachma.org/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/monitoring.drachma.org/privkey.pem;

    location /prometheus/ {
        proxy_pass http://localhost:9090/;
        auth_basic "Monitoring";
        auth_basic_user_file /etc/nginx/.htpasswd;
    }

    location / {
        proxy_pass http://localhost:3000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

## Troubleshooting

### Metrics Not Showing

```bash
# Check if node is exposing metrics
curl http://localhost:9311/metrics

# Check Prometheus targets
# Go to http://localhost:9090/targets

# Check node logs
sudo journalctl -u drachma-seed-node-1 | grep -i metric
```

### High Cardinality Issues

If Prometheus uses too much memory:

```yaml
# Add to prometheus.yml
global:
  external_labels:
    # Reduce label cardinality
    cluster: 'mainnet'
```

### Missing Alerts

```bash
# Check Alertmanager status
curl http://localhost:9093/api/v2/status

# Check alert rules
curl http://localhost:9090/api/v1/rules

# Test alert
curl -X POST http://localhost:9093/api/v1/alerts -d '[{"labels":{"alertname":"test"}}]'
```

## Advanced Topics

### Service Discovery

For dynamic node discovery, use Prometheus service discovery:

```yaml
scrape_configs:
  - job_name: 'drachma-kubernetes'
    kubernetes_sd_configs:
      - role: pod
```

### Long-term Storage

For retention beyond 30 days, use Thanos or Cortex:

```bash
# Thanos sidecar
docker run -d \
  --name thanos-sidecar \
  -v /path/to/prometheus:/prometheus \
  quay.io/thanos/thanos:latest \
  sidecar \
  --tsdb.path /prometheus \
  --prometheus.url http://prometheus:9090
```

### Automated Remediation

For common issues, automate fixes:

```yaml
# In Alertmanager
receivers:
  - name: 'webhook-remediation'
    webhook_configs:
      - url: 'http://remediation-service/api/alert'
```

## Monitoring Checklist

- [ ] Prometheus installed and running
- [ ] Grafana installed and configured
- [ ] Node metrics exposed (:9311)
- [ ] System metrics via node_exporter (:9100)
- [ ] Dashboards imported and customized
- [ ] Alert rules configured
- [ ] Alertmanager set up with notifications
- [ ] SSL/HTTPS configured for web interfaces
- [ ] Firewall rules restricting access
- [ ] Backup strategy for dashboards
- [ ] Documentation for on-call team
- [ ] Test alerts verified

## Support

For monitoring questions:
- GitHub Issues: https://github.com/Tsoympet/PARTHENON-CHAIN/issues
- Discussions: https://github.com/Tsoympet/PARTHENON-CHAIN/discussions

## Resources

- [Prometheus Documentation](https://prometheus.io/docs/)
- [Grafana Documentation](https://grafana.com/docs/)
- [Alertmanager Documentation](https://prometheus.io/docs/alerting/latest/alertmanager/)
- [Node Exporter](https://github.com/prometheus/node_exporter)
