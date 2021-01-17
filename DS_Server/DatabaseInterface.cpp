#include <string>
#include <vector>
#include "service.grpc.pb.h"


/*
class Entry {
private:
	int _global_index;
	std::string _key_value;
	std::string _key_name;
	std::string _value;
	std::string _table_name;
	bool _sort = false;

public:
	bool operator== (Entry& val) {
		bool result = (_key_name == val.key_name()) &&
			(_key_value == val.key_value()) &&
			(_value == val.value()) &&
			(_table_name == val.table_name());
		return result;
	}

	void set_global_index(int val) {
		_global_index = val;
	}

	void set_value(std::string val) {
		_value = val;
	}

	void set_table_name(std::string val) {
		_table_name = val;
	}

	void set_key_name(std::string val) {
		_key_name = val;
	}

	void set_key_value(std::string val) {
		_key_value = val;
	}

	void set_sort(bool val) {
		_sort = val;
	}

	int global_index() {
		return _global_index;
	}

	std::string value() {
		return _value;
	}

	std::string table_name() {
		return _table_name;
	}

	std::string key_name() {
		return _key_name;
	}

	std::string key_value() {
		return _key_value;
	}

	bool sort() {
		return _sort;
	}
};

class KeyValue {
private:
	std::string _name;
	std::string _value;

public:
	std::string name() {
		return _name;
	}

	std::string value() {
		return _value;
	}

	void set_name(std::string val) {
		_name = val;
	}

	void set_value(std::string val) {
		_value = val;
	}
};
*/

class DatabaseInterface {
public:
	DatabaseInterface(){}
	virtual ~DatabaseInterface(){}

	virtual bool CreateTable(std::string name, std::vector<std::string> keys) = 0;
	virtual bool DeleteTable(std::string name) = 0;

	virtual Entry GetFirstEntry(std::string name, std::string key_name) = 0;
	virtual Entry GetLastEntry(std::string name, std::string key_name) = 0;

	virtual Entry GetEntry(std::string name, std::string key_name, std::string key_value) = 0;
	virtual Entry GetNextEntry(Entry entry) = 0;
	virtual Entry GetPrevEntry(Entry entry) = 0;

	virtual bool AddEntry(std::string table_name, std::vector<KeyValue> keys, std::string value) = 0;
	virtual bool DeleteCurrentEntry(Entry entry) = 0;
};

class DatabaseStub : public DatabaseInterface {
	public:

	// Inherited via DatabaseInterface
	bool CreateTable(std::string name, std::vector<std::string> keys) override
	{
		return false;
	}

	bool DeleteTable(std::string name) override
	{
		return false;
	}
	Entry GetFirstEntry(std::string name, std::string key_name) override
	{
		return Entry();
	}
	Entry GetLastEntry(std::string name, std::string key_name) override
	{
		return Entry();
	}
	Entry GetEntry(std::string name, std::string key_name, std::string key_value) override
	{
		return Entry();
	}
	Entry GetNextEntry(Entry entry) override
	{
		return Entry();
	}
	Entry GetPrevEntry(Entry entry) override
	{
		return Entry();
	}
	bool AddEntry(std::string table_name, std::vector<KeyValue> keys, std::string value) override
	{
		return false;
	}
	bool DeleteCurrentEntry(Entry entry) override
	{
		return false;
	}
};