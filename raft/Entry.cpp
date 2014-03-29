
#include "Entry.h"

string Entry::toJSON()
{
	string result = " { ";

	result += "\"ct\"";
	result += ":";
	result += Poco::NumberFormatter::format(current_term);
	result += ",";

	result += "\"cc\"";
	result += ":";
	result += Poco::NumberFormatter::format(current_committed);
	result += ",";

	result += "\"ci\"";
	result += ":";
	result += Poco::NumberFormatter::format(current_index);
	result += ",";

	result += "\"cm\"";
	result += ":";
	result += "\"" + command + "\"";

	result += " }"
	return result;
}

string Entry::combineToJSON(Entry &entry)
{
	string result = " { ";

	result += "\"entities\"";
	result += ":";
	result += "[";
	
	//first
	result += "{"
	result += "\"ct\"";
	result += ":";
	result += Poco::NumberFormatter::format(current_term);
	result += ",";

	result += "\"cc\"";
	result += ":";
	result += Poco::NumberFormatter::format(current_committed);
	result += ",";

	result += "\"ci\"";
	result += ":";
	result += Poco::NumberFormatter::format(current_index);
	result += ",";

	result += "\"cm\"";
	result += ":";
	result += "\"" + command + "\"";
	result += "},";

	//second
	result += "{"
	result += "\"ct\"";
	result += ":";
	result += Poco::NumberFormatter::format(entry.getCurrentTerm());
	result += ",";

	result += "\"cc\"";
	result += ":";
	result += Poco::NumberFormatter::format(entry.getCurrentCommitted());
	result += ",";

	result += "\"ci\"";
	result += ":";
	result += Poco::NumberFormatter::format(entry.getCurrentIndex());
	result += ",";

	result += "\"cm\"";
	result += ":";
	result += "\"" + entry.getCommand() + "\"";
	result += "}"

	result += "]";
	result += " }"

	return result;
}

int Entry::getCurrentTerm()
{
	return current_term;
}

int Entry::getCurrentCommitted()
{
	return current_committed;
}

int Entry::getCommand()
{
	return command;
}

int Entry::getCurrentIndex()
{
	return current_index;
}


