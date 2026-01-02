add_test([=[Integration.RegtestHarness]=]  /home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/regtest_mode_test [==[--gtest_filter=Integration.RegtestHarness]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Integration.RegtestHarness]=]  PROPERTIES WORKING_DIRECTORY /home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  regtest_mode_test_TESTS Integration.RegtestHarness)
