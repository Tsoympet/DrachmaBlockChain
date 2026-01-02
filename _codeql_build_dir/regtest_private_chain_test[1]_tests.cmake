add_test([=[Integration.PrivateRegtestChainProgression]=]  /home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/regtest_private_chain_test [==[--gtest_filter=Integration.PrivateRegtestChainProgression]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Integration.PrivateRegtestChainProgression]=]  PROPERTIES WORKING_DIRECTORY /home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  regtest_private_chain_test_TESTS Integration.PrivateRegtestChainProgression)
