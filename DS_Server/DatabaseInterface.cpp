#include <string>

#include "service.grpc.pb.h";

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