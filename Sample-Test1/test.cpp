//#include "pch.h"
#include "gtest/gtest.h"
#include "service.grpc.pb.cc"
#include "service.pb.cc"
#include "Database.cpp"


using namespace std;

TEST(TestCaseName1, TestName1)
{
	DatabaseInterface* database = new Database();
	vector<string> keys{ "key1", "key2" };
	EXPECT_EQ(true, database->CreateTable("table1", keys));
}

TEST(TestCaseName2, TestName2)
{
	DatabaseInterface* database = new Database();
	vector<string> keyss{ "key1", "key2" };
	vector<KeyValue> keys;
	KeyValue key_value;
	key_value.set_name("key1");
	key_value.set_value("value1_1");
	keys.push_back(key_value);
	database->CreateTable("table1", keyss);
	EXPECT_EQ(true, database->AddEntry("table1", keys, "my_value_1"));
}

TEST(TestCaseName3, TestName3)
{
	DatabaseInterface* database = new Database();
	vector<string> keyss{ "key1", "key2" };
	vector<KeyValue> keys;
	KeyValue key_value;
	key_value.set_name("key1");
	key_value.set_value("value1_1");
	keys.push_back(key_value);
	database->CreateTable("table1", keyss);
	database->AddEntry("table1", keys, "my_value_1");

    Entry return_entry = database->GetFirstEntry("table1", "key1", false);

	Entry entry;
	entry.set_value("my_value_1");
	entry.set_table_name("table1");
	entry.set_key_name("key1");
	entry.set_key_value("value1_1");
	entry.set_sort(true);

	bool result = return_entry.value() == entry.value();

	EXPECT_EQ(true, result);
}

TEST(TestCaseName4, TestName4)
{
	DatabaseInterface* database = new Database();
	vector<string> keyss{ "key1", "key2" };
	database->CreateTable("table1", keyss);

	vector<KeyValue> keys;
	KeyValue key_value;
	key_value.set_name("key1");
	key_value.set_value("value1_1");
	keys.push_back(key_value);
	database->AddEntry("table1", keys, "my_value_1");

	vector<KeyValue> keys2;
	KeyValue key_value2;
	key_value2.set_name("key1");
	key_value2.set_value("value1_2");
	keys2.push_back(key_value2);
	database->AddEntry("table1", keys2, "my_value_2");

	vector<KeyValue> keys3;
	KeyValue key_value3;
	key_value3.set_name("key1");
	key_value3.set_value("value1_3");
	keys3.push_back(key_value3);
	database->AddEntry("table1", keys3, "my_value_3");

	Entry return_entry = database->GetLastEntry("table1", "key1", true);
	Entry next_entry = database->GetPrevEntry(return_entry);

	Entry entry;
	entry.set_value("my_value_1");
	entry.set_table_name("table1");
	entry.set_key_name("key1");
	entry.set_key_value("value1_1");
	entry.set_sort(true);

	bool result = next_entry.key_value() == entry.key_value() &&
		next_entry.value() == entry.value() &&
		next_entry.key_name() == entry.key_name();

	EXPECT_EQ(true, result);
}