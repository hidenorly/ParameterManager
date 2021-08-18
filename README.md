# ParameterManager

C++ based Parameter Manager

```
clang++ -stdlib=libc++ -std=c++2a -MMD -MP -Wall -pthread -lgtest_main -lgtest ParameterManager.cpp Stream.cpp StringTokenizer.cpp testcase.cpp
```

# basic set and get and callback

```
  std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();

  ParameterManager::CALLBACK callbackW = [](std::string key, std::string value){
    std::cout << "callback(param*)): [" << key << "] = " << value << std::endl;
  };
  int callbackIdW = pParams->registerCallback("param*", callbackW);

  pParams->setParameter("paramA", "ABC");
  EXPECT_TRUE( pParams->getParameter("paramA", "HOGE") == "ABC" );

  pParams->setParameterBool("paramB", true);
  EXPECT_TRUE( pParams->getParameterBool("paramB", false) == true );

  pParams->setParameterInt("paramC", 1);
  EXPECT_TRUE( pParams->getParameterInt("paramC", 0) == 1 );
  EXPECT_TRUE( pParams->getParameterInt("paramD", -1) == -1 );
```

# save

```
  std::cout << "store to stream" << std::endl;
  FileStream* pFileStream = new FileStream( paramFilePath );
  pParams->storeToStream( pFileStream );
  pFileStream->close();
```

# reset

```
  std::cout << "reset all of params" << std::endl;
  pParams->resetAllOfParams();
```

# load

```
  std::cout << "restore from stream" << std::endl;
  pFileStream = new FileStream( paramFilePath );
  pParams->restoreFromStream( pFileStream );
  pFileStream->close();
```

# rule

```
  std::shared_ptr<ParameterManager> pParams = ParameterManager::getManager().lock();
  pParams->resetAllOfParams();

  // --- int, range
  pParams->setParameterRule( "paramA",
    ParameterManager::ParamRule(
      ParameterManager::ParamType::TYPE_INT,
      -12, 12) );

  // out of range
  pParams->setParameterInt("paramA", -13);
  EXPECT_EQ( pParams->getParameterInt("paramA", 0), -12 );

  // in range
  pParams->setParameterInt("paramA", -10);
  EXPECT_EQ( pParams->getParameterInt("paramA", 0), -10 );
```

```
  // enum string
  pParams->setParameterRule( "paramC",
    ParameterManager::ParamRule(
      ParameterManager::ParamType::TYPE_STRING,
      {"LOW", "MID", "HIGH"}) );

  // ng case : enum
  pParams->setParameterInt("paramC", -13);
  EXPECT_EQ( pParams->getParameter("paramC", "LOW"), "LOW" );

  // ok case : enum
  pParams->setParameter("paramC", "HIGH");
  EXPECT_EQ( pParams->getParameter("paramC", "LOW"), "HIGH" );

  // rule
  ParameterManager::ParamRule ruleC = pParams->getParameterRule("paramC");
  EXPECT_EQ( ruleC.type, ParameterManager::ParamType::TYPE_STRING );
  EXPECT_EQ( ruleC.range, ParameterManager::ParamRange::RANGE_ENUM );
  EXPECT_EQ( ruleC.enumVals, std::vector<std::string>({"LOW", "MID", "HIGH"}));
```

See more about the test case

# test result

```
$ ./a.out 
[==========] Running 2 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 2 tests from TestCase_System
[ RUN      ] TestCase_System.testParameterManager
callback(param*)): [paramA] = ABC
callback(param*)): [paramB] = true
callback(param*)): [paramC] = 1
callback(param*)): [paramC] = 2
callback(exact match): [paramC] = 2
callback(param*)): [paramC] = 3
callback(exact match): [paramC] = 3
unregistered all notifier
getParameters()
paramA = ABC
paramB = true
paramC = 4
ro.paramD = 1

getParameters("param*")
paramA = ABC
paramB = true
paramC = 4

store to stream
reset all of params
restore from stream
reset all of params
restore from stream
paramA = XXX
paramB = true
paramC = 4
ro.paramD = 1
[       OK ] TestCase_System.testParameterManager (1 ms)
[ RUN      ] TestCase_System.testParameterManagerRule
[       OK ] TestCase_System.testParameterManagerRule (0 ms)
[----------] 2 tests from TestCase_System (1 ms total)

[----------] Global test environment tear-down
[==========] 2 tests from 1 test suite ran. (1 ms total)
[  PASSED  ] 2 tests.
```