#!/usr/bin/env python3
"""
Extended validation script for PARTHENON CHAIN
Tests network stability over extended periods (7+ days)

Usage:
    python3 scripts/extended-testnet-validation.py --duration 7 --nodes http://node1:18332,http://node2:18332
"""

import argparse
import time
import json
import requests
import sys
from datetime import datetime, timedelta
from collections import defaultdict
import logging

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class NetworkMonitor:
    def __init__(self, nodes, username, password, interval=60):
        self.nodes = nodes
        self.auth = (username, password)
        self.interval = interval
        self.metrics = defaultdict(list)
        self.start_time = datetime.now()
        
    def rpc_call(self, node_url, method, params=None):
        """Make RPC call to a node
        
        Note: This sends credentials in plaintext. For production use,
        ensure the connection is over a trusted network or use SSL/TLS.
        """
        try:
            payload = {
                "jsonrpc": "2.0",
                "id": "extended-validation",
                "method": method,
                "params": params or []
            }
            resp = requests.post(node_url, json=payload, auth=self.auth, timeout=10)
            resp.raise_for_status()
            result = resp.json()
            if result.get("error"):
                logger.error(f"RPC error from {node_url}: {result['error']}")
                return None
            return result.get("result")
        except Exception as e:
            logger.error(f"Failed to call {method} on {node_url}: {e}")
            return None
    
    def collect_metrics(self):
        """Collect metrics from all nodes"""
        timestamp = datetime.now()
        
        for node_url in self.nodes:
            logger.info(f"Collecting metrics from {node_url}")
            
            # Get blockchain info
            info = self.rpc_call(node_url, "getblockchaininfo")
            if info:
                self.metrics[f"{node_url}_height"].append({
                    "time": timestamp,
                    "value": info.get("blocks", 0)
                })
                self.metrics[f"{node_url}_verification_progress"].append({
                    "time": timestamp,
                    "value": info.get("verificationprogress", 0)
                })
            
            # Get network info
            net_info = self.rpc_call(node_url, "getnetworkinfo")
            if net_info:
                self.metrics[f"{node_url}_connections"].append({
                    "time": timestamp,
                    "value": net_info.get("connections", 0)
                })
            
            # Get peer info
            peers = self.rpc_call(node_url, "getpeerinfo")
            if peers:
                self.metrics[f"{node_url}_peer_count"].append({
                    "time": timestamp,
                    "value": len(peers)
                })
            
            # Get mempool info
            mempool = self.rpc_call(node_url, "getmempoolinfo")
            if mempool:
                self.metrics[f"{node_url}_mempool_size"].append({
                    "time": timestamp,
                    "value": mempool.get("size", 0)
                })
                self.metrics[f"{node_url}_mempool_bytes"].append({
                    "time": timestamp,
                    "value": mempool.get("bytes", 0)
                })
            
            # Get mining info
            mining = self.rpc_call(node_url, "getmininginfo")
            if mining:
                self.metrics[f"{node_url}_difficulty"].append({
                    "time": timestamp,
                    "value": mining.get("difficulty", 0)
                })
    
    def check_sync_status(self):
        """Check if all nodes are in sync"""
        heights = []
        for node_url in self.nodes:
            info = self.rpc_call(node_url, "getblockchaininfo")
            if info:
                heights.append((node_url, info.get("blocks", 0)))
        
        if not heights:
            logger.error("Could not get height from any node")
            return False
        
        # Check if heights are within 1 block of each other
        min_height = min(h[1] for h in heights)
        max_height = max(h[1] for h in heights)
        
        if max_height - min_height > 1:
            logger.warning(f"Nodes out of sync! Heights: {heights}")
            return False
        
        logger.info(f"All nodes in sync at height {max_height}")
        return True
    
    def check_fork_detection(self):
        """Check for potential chain forks"""
        block_hashes = []
        
        # Get the latest block hash from each node
        for node_url in self.nodes:
            info = self.rpc_call(node_url, "getblockchaininfo")
            if info:
                height = info.get("blocks", 0)
                best_hash = info.get("bestblockhash")
                block_hashes.append((node_url, height, best_hash))
        
        if len(set(h[2] for h in block_hashes)) > 1:
            logger.error(f"FORK DETECTED! Different best blocks: {block_hashes}")
            return False
        
        return True
    
    def stress_test_mempool(self):
        """Stress test by creating transactions"""
        logger.info("Running mempool stress test...")
        
        for node_url in self.nodes:
            # Get new address
            addr = self.rpc_call(node_url, "getnewaddress")
            if not addr:
                continue
            
            # Try to send small transaction
            try:
                txid = self.rpc_call(node_url, "sendtoaddress", [addr, 0.001])
                if txid:
                    logger.info(f"Created test transaction: {txid[:16]}...")
            except Exception as e:
                logger.warning(f"Could not create test transaction: {e}")
    
    def analyze_metrics(self):
        """Analyze collected metrics for issues"""
        issues = []
        
        # Check for nodes going offline
        for node_url in self.nodes:
            height_key = f"{node_url}_height"
            if height_key in self.metrics:
                recent = self.metrics[height_key][-10:]  # Last 10 samples
                if len(recent) < 5:
                    issues.append(f"Node {node_url} has insufficient data points")
                elif all(m["value"] == recent[0]["value"] for m in recent):
                    issues.append(f"Node {node_url} appears stuck at height {recent[0]['value']}")
        
        # Check for peer count drops
        for node_url in self.nodes:
            peer_key = f"{node_url}_peer_count"
            if peer_key in self.metrics:
                recent = self.metrics[peer_key][-10:]
                if recent and all(m["value"] < 3 for m in recent):
                    issues.append(f"Node {node_url} has consistently low peer count")
        
        return issues
    
    def generate_report(self, output_file):
        """Generate validation report"""
        duration = datetime.now() - self.start_time
        
        report = {
            "test_start": self.start_time.isoformat(),
            "test_end": datetime.now().isoformat(),
            "duration_seconds": duration.total_seconds(),
            "duration_days": duration.days,
            "nodes_tested": self.nodes,
            "metrics_collected": {k: len(v) for k, v in self.metrics.items()},
            "issues_found": self.analyze_metrics(),
            "summary": {
                "total_samples": sum(len(v) for v in self.metrics.values()),
                "test_passed": len(self.analyze_metrics()) == 0
            }
        }
        
        with open(output_file, 'w') as f:
            json.dump(report, f, indent=2)
        
        logger.info(f"Report saved to {output_file}")
        return report
    
    def run(self, duration_days):
        """Run extended validation for specified duration"""
        end_time = self.start_time + timedelta(days=duration_days)
        
        logger.info(f"Starting {duration_days}-day validation test")
        logger.info(f"Testing {len(self.nodes)} nodes")
        logger.info(f"End time: {end_time}")
        
        iteration = 0
        try:
            while datetime.now() < end_time:
                iteration += 1
                logger.info(f"=== Iteration {iteration} ===")
                
                # Collect metrics
                self.collect_metrics()
                
                # Check sync status
                in_sync = self.check_sync_status()
                
                # Check for forks
                no_fork = self.check_fork_detection()
                
                # Periodic stress test (every 6 hours)
                if iteration % 360 == 0:
                    self.stress_test_mempool()
                
                # Log progress
                elapsed = datetime.now() - self.start_time
                remaining = end_time - datetime.now()
                logger.info(f"Elapsed: {elapsed}, Remaining: {remaining}")
                
                # Sleep until next iteration
                time.sleep(self.interval)
                
        except KeyboardInterrupt:
            logger.info("Test interrupted by user")
        finally:
            # Generate final report
            report = self.generate_report("extended-validation-report.json")
            
            # Print summary
            print("\n" + "="*60)
            print("EXTENDED VALIDATION TEST SUMMARY")
            print("="*60)
            print(f"Duration: {report['duration_days']} days")
            print(f"Total samples: {report['summary']['total_samples']}")
            print(f"Issues found: {len(report['issues_found'])}")
            
            if report['issues_found']:
                print("\nIssues:")
                for issue in report['issues_found']:
                    print(f"  - {issue}")
            
            if report['summary']['test_passed']:
                print("\n✅ TEST PASSED - Network is stable")
                return 0
            else:
                print("\n❌ TEST FAILED - Issues detected")
                return 1


def main():
    parser = argparse.ArgumentParser(
        description='Extended validation for PARTHENON CHAIN testnet'
    )
    parser.add_argument(
        '--duration',
        type=float,
        default=7.0,
        help='Test duration in days (default: 7)'
    )
    parser.add_argument(
        '--nodes',
        required=True,
        help='Comma-separated list of node RPC URLs'
    )
    parser.add_argument(
        '--username',
        default='user',
        help='RPC username'
    )
    parser.add_argument(
        '--password',
        default='pass',
        help='RPC password'
    )
    parser.add_argument(
        '--interval',
        type=int,
        default=60,
        help='Sampling interval in seconds (default: 60)'
    )
    
    args = parser.parse_args()
    
    # Parse node URLs
    nodes = [url.strip() for url in args.nodes.split(',')]
    
    # Create monitor and run test
    monitor = NetworkMonitor(nodes, args.username, args.password, args.interval)
    exit_code = monitor.run(args.duration)
    
    sys.exit(exit_code)


if __name__ == '__main__':
    main()
