#include <map>
#include <vector>
#include <iostream>

#include "DatabaseInterface.cpp"
#include <algorithm>

class Database : public DatabaseInterface {

	class Table
	{
		struct IndexedEntry
		{
			int index;
			Entry entry;

			IndexedEntry() {
				index = 0;
				entry = Entry();
			}
		};

		std::string table_name = "New table";

		// index:value
		std::map<int, std::string> main_table;
		std::map<int, std::vector<std::pair<std::string, std::string>>> main_keys_table;

		// key_name: hash_table<key_value, indexes>
		std::map<std::string, std::map<std::string, std::vector<int>>> keys_map;

		std::string error_message = "undefined_error";
		Entry errorEntry;
		IndexedEntry errorIndexedEntry;

		int global_index;

	public:
		Table() { };
		Table(std::string name, std::vector<std::string> keys)
		{
			table_name = name;
			global_index = 1000;

			for (std::string key : keys) {
				std::map<std::string, std::vector<int>> empty_map;
				keys_map[key] = empty_map;
			}

			errorEntry.set_value("Error");
			errorEntry.set_table_name("Error message");

			errorIndexedEntry.index = -1;
			errorIndexedEntry.entry = errorEntry;
		};
		~Table() { };

		std::string GetErrorMessage() {
			return error_message;
		}

		Entry GetFirstEntry(std::string key_name) {
			if (HasKey(key_name)) {
				return GetEntrySorted(true, key_name);
			}

			errorEntry.set_table_name("Error. No key found with name [" + key_name + "]");
			return errorEntry;
		}


		Entry GetLastEntry(std::string key_name) {
			if (HasKey(key_name)) {
				return GetEntrySorted(false, key_name);
			}

			errorEntry.set_table_name("Error. No key found with name [" + key_name + "]");
			return errorEntry;
		}

		Entry GetEntry(std::string key_name, std::string key_value) {
			return GetIndexedEntry(key_name, key_value).entry;
		}

		Entry GetNextEntry(Entry entry) {
			if (HasKey(entry.key_name())) {
				auto iterator = keys_map[entry.key_name()].find(entry.key_value());
				if (iterator != keys_map[entry.key_name()].end()) {
					
					//Check same key names first
					if ((*iterator).second.size() > 1) {
						std::vector<int> indexes = (*iterator).second;
						if (indexes[indexes.size() - 1] != entry.global_index()) {
							auto itemIndex = std::find(indexes.begin(), indexes.end(), entry.global_index());
							itemIndex++;
							int new_index = (*itemIndex);
							return ReturnIndexedEntry(new_index, main_table[new_index], entry.key_name(), entry.key_value(), true).entry;
						}
					}

					iterator++;
					if (iterator == keys_map[entry.key_name()].end())
						iterator = keys_map[entry.key_name()].begin();

					int index = (*iterator).second[0];
					std::string key_value = (*iterator).first;
					std::string value = main_table[index];
					
					return ReturnIndexedEntry(index, value, entry.key_name(), key_value, true).entry;
				}

				errorEntry.set_table_name("Error. No key value found with key [" + entry.key_value() + "]");
				return errorEntry;
			}

			errorEntry.set_table_name("Error. No key found with name [" + entry.key_name() + "]");
			return errorEntry;
		}

		Entry GetPrevEntry(Entry entry) {
			if (HasKey(entry.key_name())) {
				auto iterator = keys_map[entry.key_name()].find(entry.key_value());
				if (iterator != keys_map[entry.key_name()].end()) {

					//Check same key names first
					if ((*iterator).second.size() > 1) {
						std::vector<int> indexes = (*iterator).second;
						if (indexes[0] != entry.global_index()) {
							auto itemIndex = std::find(indexes.begin(), indexes.end(), entry.global_index());
							itemIndex--;
							int new_index = (*itemIndex);
							return ReturnIndexedEntry(new_index, main_table[new_index], entry.key_name(), entry.key_value(), true).entry;
						}
					}

					if (iterator == keys_map[entry.key_name()].begin())
						iterator = keys_map[entry.key_name()].end();
					iterator--;

					int index = (*iterator).second[0];
					std::string key_value = (*iterator).first;
					std::string value = main_table[index];

					return ReturnIndexedEntry(index, value, entry.key_name(), key_value, true).entry;
				}

				errorEntry.set_table_name("Error. No key value found with key [" + entry.key_value() + "]");
				return errorEntry;
			}

			errorEntry.set_table_name("Error. No key found with name [" + entry.key_name() + "]");
			return errorEntry;
		}

		bool CreateEntry(std::vector<KeyValue> keys, std::string value) {
			main_table[global_index] = value;
			main_keys_table[global_index] = std::vector<std::pair<std::string, std::string>>();
			for (KeyValue key_pair : keys) {
				if (!HasKey(key_pair.name())) {
					error_message = "Error. No key found with name [" + key_pair.name() + "]";
					return false;
				}
				if (keys_map[key_pair.name()].find(key_pair.value()) == keys_map[key_pair.name()].end()) {
					keys_map[key_pair.name()][key_pair.value()] = std::vector<int>();
				}
				keys_map[key_pair.name()][key_pair.value()].push_back(global_index);
				main_keys_table[global_index].push_back(std::make_pair(key_pair.name(), key_pair.value()));
			}
			global_index++;
			return true;
		}

		bool DeleteEntry(Entry entry) {
			if (!HasKey(entry.key_name())) {
				error_message = "Error. No key found with name [" + entry.key_name() + "]";
				return false;
			}

			if (keys_map[entry.key_name()].find(entry.key_value()) != keys_map[entry.key_name()].end()) {
				IndexedEntry entryToDelete = GetIndexedEntry(entry.key_name(), entry.key_value());
				main_table.erase(entryToDelete.index);
				for (std::pair<std::string, std::string> key_pair : main_keys_table[entryToDelete.index]) {
					std::vector<int> indexes = keys_map[key_pair.first][key_pair.second];
					indexes.erase(std::remove(indexes.begin(), indexes.end(), entryToDelete.index), indexes.end());
					
					if (indexes.size() == 0) {
						keys_map[key_pair.first].erase(key_pair.second);
					}
					else {
						keys_map[key_pair.first][key_pair.second] = indexes;
					}
				}

				return true;
			}

			error_message = "Error. No key value found with name [" + entry.key_value() + "]";
			return false;
		}

	private:
		IndexedEntry GetIndexedEntry(std::string key_name, std::string key_value) {
			if (HasKey(key_name)) {
				if (keys_map[key_name].find(key_value) != keys_map[key_name].end()) {
					if (keys_map[key_name][key_value].size() > 0) {
						int index = keys_map[key_name][key_value][0];
						std::string value = main_table[index];
						return ReturnIndexedEntry(index, value, key_name, key_value, true);
					}

					errorIndexedEntry.entry.set_table_name("Error. No value found with key name [" + key_value + "]");
					return errorIndexedEntry;
				}

				errorIndexedEntry.entry.set_table_name("Error. No key value found with key [" + key_value + "]");
				return errorIndexedEntry;
			}

			errorIndexedEntry.entry.set_table_name("Error. No key found with name [" + key_name + "]");
			return errorIndexedEntry;
		}

		Entry GetEntrySorted(bool sort, std::string key_name) {
			std::vector<std::string> keys_values;
			for (auto it = keys_map[key_name].begin(); it != keys_map[key_name].end(); ++it) {
				keys_values.push_back(it->first);
			}

			if (keys_values.empty()) {
				errorEntry.set_table_name("Error. No keys values found with key [" + key_name + "]");
				return errorEntry;
			}

			std::sort(keys_values.begin(), keys_values.end());
			std::string key_value = sort ? keys_values.front() : keys_values.back();
			int index = keys_map[key_name][key_value][0];
			std::string value = main_table[index];
			return ReturnIndexedEntry(index, value, key_name, key_value, sort).entry;
		}

		IndexedEntry ReturnIndexedEntry(int index, std::string value, std::string key_name, std::string key_value, bool sort) {
			Entry entry;
			entry.set_global_index(index);
			entry.set_value(value);
			entry.set_table_name(table_name);
			entry.set_key_name(key_name);
			entry.set_key_value(key_value);
			entry.set_sort(sort);

			IndexedEntry indexedEntry;
			indexedEntry.index = index;
			indexedEntry.entry = entry;

			return indexedEntry;
		}

		bool HasKey(std::string key_name) {
			return keys_map.find(key_name) != keys_map.end();
		}

	};

	// all tables
	std::map<std::string, Table> tables;

public:
	std::string error_message = "undefined_error";
	Entry error_entry;

	Database() {
		error_entry.set_value("Error");
	}

	bool CreateTable(std::string name, std::vector<std::string> keys) override
	{
		if (HasTable(name)) {
			return false;
		}

		tables[name] = Table(name, keys);
		return true;
	}

	bool DeleteTable(std::string name) override
	{
		if (!HasTable(name)) {
			error_message = "Error. Table was not found.";
			return false;
		}
		tables.erase(name);
		return true;
	}

	Entry GetFirstEntry(std::string name, std::string key_name, bool sort_order) override
	{
		if (!HasTable(name)) {
			error_message = "Error. Table was not found.";
			return error_entry;
		}

		return sort_order ? tables[name].GetLastEntry(key_name) : tables[name].GetFirstEntry(key_name);
	}

	Entry GetLastEntry(std::string name, std::string key_name, bool sort_order) override
	{
		if (!HasTable(name)) {
			error_message = "Error. Table was not found.";
			return error_entry;
		}
		return sort_order ? tables[name].GetFirstEntry(key_name) : tables[name].GetLastEntry(key_name);
	}

	Entry GetEntry(std::string name, std::string key_name, std::string key_value) override
	{
		if (!HasTable(name)) {
			error_message = "Error. Table was not found.";
			return error_entry;
		}
		return tables[name].GetEntry(key_name, key_value);
	}

	Entry GetNextEntry(Entry entry) override
	{
		return entry.sort() ? tables[entry.table_name()].GetNextEntry(entry) : tables[entry.table_name()].GetPrevEntry(entry);
	}

	Entry GetPrevEntry(Entry entry) override
	{
		return entry.sort() ? tables[entry.table_name()].GetPrevEntry(entry) : tables[entry.table_name()].GetNextEntry(entry);
	}

	bool AddEntry(std::string table_name, std::vector<KeyValue> keys, std::string value) override
	{
		if (!HasTable(table_name)) {
			error_message = "Error. Table was not found.";
			return false;
		}
		return tables[table_name].CreateEntry(keys, value);
	}

	bool DeleteCurrentEntry(Entry entry) override
	{
		bool result = tables[entry.table_name()].DeleteEntry(entry);

		if (!result) {
			error_message = tables[entry.table_name()].GetErrorMessage();
		}

		return result;
	}

	std::string GetErrorString() override {
		return error_message;
	}

private:

	bool HasTable(std::string table_name) {
		return tables.find(table_name) != tables.end();
	}
};