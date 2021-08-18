/*
  Copyright (C) 2021 hidenorly

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <gtest/gtest.h>
#include "testcase.hpp"
#include "ParameterManager.hpp"

TestCase_System::TestCase_System()
{
}

TestCase_System::~TestCase_System()
{
}

void TestCase_System::SetUp()
{
}

void TestCase_System::TearDown()
{
}

TEST_F(TestCase_System, testParameterManager)
{
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

  std::vector<std::string> keys = {"paramA", "paramB", "paramC"};
  std::vector<ParameterManager::Param> params = pParams->getParameters(keys);
  EXPECT_EQ( params.size(), 3 );

  pParams->setParameterInt("ro.paramD", 1);
  EXPECT_TRUE( pParams->getParameterInt("ro.paramD", 0) == 1 );
  pParams->setParameterInt("ro.paramD", 2);
  EXPECT_TRUE( pParams->getParameterInt("ro.paramD", 0) == 1 );

  ParameterManager::CALLBACK callback2 = [](std::string key, std::string value){
    std::cout << "callback(exact match): [" << key << "] = " << value << std::endl;
  };
  int callbackId1 = pParams->registerCallback("paramC", callback2);
  int callbackId2 = pParams->registerCallback("ro.paramD", callback2);
  pParams->setParameterInt("paramC", 1);
  pParams->setParameterInt("paramC", 2);
  pParams->setParameterInt("paramC", 3);
  pParams->setParameterInt("ro.paramD", 3);

  pParams->unregisterCallback(callbackIdW);
  pParams->unregisterCallback(callbackId1);
  pParams->unregisterCallback(callbackId2);
  pParams->unregisterCallback(10000);
  std::cout << "unregistered all notifier" << std::endl;
  pParams->setParameterInt("paramC", 4);

  // dump all
  std::cout << "getParameters()" << std::endl;
  std::vector<ParameterManager::Param> paramsAll = pParams->getParameters();
  for(auto& aParam : paramsAll){
    std::cout << aParam.key << " = " << aParam.value << std::endl;
  }
  std::cout << std::endl;

  std::cout << "getParameters(\"param*\")" << std::endl;
  std::vector<ParameterManager::Param> paramsWilds = pParams->getParameters("param*");
  for(auto& aParam : paramsWilds){
    std::cout << aParam.key << " = " << aParam.value << std::endl;
  }
  std::cout << std::endl;

  const std::string paramFilePath = "TestProperties";

  if( std::filesystem::exists( paramFilePath) ){
    std::filesystem::remove( paramFilePath );
  }

  std::cout << "store to stream" << std::endl;
  FileStream* pFileStream = new FileStream( paramFilePath );
  pParams->storeToStream( pFileStream );
  pFileStream->close();

  std::cout << "reset all of params" << std::endl;
  pParams->resetAllOfParams();

  std::cout << "restore from stream" << std::endl;
  pFileStream = new FileStream( paramFilePath );
  pParams->restoreFromStream( pFileStream );
  pFileStream->close();

  std::cout << "reset all of params" << std::endl;
  pParams->resetAllOfParams();

  // non-override restore. This helps to implement default params and user params load. Load current user config value(override=true) and Load the default(preset) value (override=false).
  pParams->setParameter("paramA", "XXX");
  std::cout << "restore from stream" << std::endl;
  pFileStream = new FileStream( paramFilePath );
  pParams->restoreFromStream( pFileStream, false ); // no override
  pFileStream->close();
  EXPECT_EQ( pParams->getParameter("paramA"), "XXX");

  paramsAll = pParams->getParameters();
  for(auto& aParam : paramsAll){
    std::cout << aParam.key << " = " << aParam.value << std::endl;
  }
}

TEST_F(TestCase_System, testParameterManagerRule)
{
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

  // out of range
  pParams->setParameterInt("paramA", 13);
  EXPECT_EQ( pParams->getParameterInt("paramA", 0), 12 );

  // rule
  ParameterManager::ParamRule ruleA = pParams->getParameterRule("paramA");
  EXPECT_EQ( ruleA.type, ParameterManager::ParamType::TYPE_INT );
  EXPECT_EQ( ruleA.range, ParameterManager::ParamRange::RANGED );
  EXPECT_EQ( (int)ruleA.rangeMin, -12 );
  EXPECT_EQ( (int)ruleA.rangeMax, 12 );

  // --- float, range
  pParams->setParameterRule( "paramF",
    ParameterManager::ParamRule(
      ParameterManager::ParamType::TYPE_FLOAT,
      0.0f, 100.0f) );

  // out of range and get with different type
  pParams->setParameterInt("paramF", -1.0f);
  EXPECT_EQ( pParams->getParameterInt("paramF", 0), 0 );

  // in range and get with defined type
  pParams->setParameterFloat("paramF", 10.0f);
  EXPECT_EQ( pParams->getParameterFloat("paramF", 0.0f), 10.0f );

  // set in range with different type and get with defined type
  pParams->setParameterInt("paramF", 20);
  EXPECT_EQ( pParams->getParameterFloat("paramF", 0.0f), 20.0f );

  // out of range
  pParams->setParameterFloat("paramF", 120.0f);
  EXPECT_EQ( pParams->getParameterInt("paramF", 0), 100.0f );

  // illegal type
  pParams->setParameter("paramF", "120.0f");
  EXPECT_EQ( pParams->getParameterInt("paramF", 0), 100.0f );

  // rule
  ParameterManager::ParamRule ruleF = pParams->getParameterRule("paramF");
  EXPECT_EQ( ruleF.type, ParameterManager::ParamType::TYPE_FLOAT );
  EXPECT_EQ( ruleF.range, ParameterManager::ParamRange::RANGED );
  EXPECT_EQ( ruleF.rangeMin, 0.0f );
  EXPECT_EQ( ruleF.rangeMax, 100.0f );

  // --- enum int
  pParams->setParameterRule( "paramB",
    ParameterManager::ParamRule(
      ParameterManager::ParamType::TYPE_INT,
      {"0", "50", "100"}) );

  // ng case : enum
  pParams->setParameterInt("paramB", -13);
  EXPECT_EQ( pParams->getParameterInt("paramB", 0), 0 );

  // ok case : enum
  pParams->setParameterInt("paramB", 50);
  EXPECT_EQ( pParams->getParameterInt("paramB", 0), 50 );

  // rule
  ParameterManager::ParamRule ruleB = pParams->getParameterRule("paramB");
  EXPECT_EQ( ruleB.type, ParameterManager::ParamType::TYPE_INT );
  EXPECT_EQ( ruleB.range, ParameterManager::ParamRange::RANGE_ENUM );
  EXPECT_EQ( ruleB.enumVals, std::vector<std::string>({"0", "50", "100"}));

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
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
