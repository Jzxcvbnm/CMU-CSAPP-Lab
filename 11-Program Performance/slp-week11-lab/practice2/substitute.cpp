/* substitute -- substitute strings in a list of files

  This program operates on a set of files listed on 
  the command line. The first file specifies a list of
  string substitutions to be performed on the remaining
  files. The list of string substitutions has the form:

  "string 1" "replacement 1"
  "string 2" "replacement 2"
  ...

  If a string contains a double quote character or
  a backslash character, escape the character with 
  backslash: "\"" denotes the string with one double
  quote character. "\\" contains one backslash.
  Each file is searched for instances of "string 1".
  Any occurences are replaced with "replacement 1".
  In a similar manner, all "string 2"s are replaced
  with "replacement 2"s, and so on.

  The results are written to the input file. Be sure
  to keep a backup of files if you do not want to lose
  the originals when you run this program.
*/

#include "fx.h"
#include "iostream.h"

// parse a quoted string from buffer
// return final index in string
int parse1(CString *buffer, int start, CString *str)
{
	// look for initial quote:
    int i = buffer->Find("\"", start);
	if (i != -1) {
		// copy to result string
		str->Empty();
		int j = 0;	// index into str
		i++; // skip over the opening double-quote
		// scan and copy up to the closing double-quote:
		while ((*buffer)[i] != 0) {
			if ((*buffer)[i] == '\\') {
				// read next char to see what to do
				i++;
				if ((*buffer)[i] != 0) {
					str->Insert(j++, CString((*buffer)[i]));
				}
			} else if ((*buffer)[i] == '\"') {
				return i + 1;
			}
			str->Insert(j++, CString((*buffer)[i]));
			i++;
		}
	}
	return -1;
}


// parse two quoted strings from buffer; return false on failure
//
bool parse(CString *buffer, CString *pattern, CString *replacement)
{
	int start = parse1(buffer, 0, pattern);
	if (start < 0) {
		return false;
	}
	start = parse1(buffer, start, replacement);
	return (start >= 0);
}



void substitute(CString *data, CString *pattern, CString *replacement)
{
	int loc;
	int location = 0;        
	int pl = pattern->Length; 
	// find every occurrence of pattern:
	for (loc = data->Find(*pattern, 0); loc >= 0;
		 loc = data->Find(*pattern, 0)) {
			int dis = loc - pl;
			if (dis >= 0){
				location = dis;
			}
			else{
				location = 0;
			}
	    // delete the pattern string from loc:
		data->Delete(loc, pattern->GetLength());
		// insert each character of the replacement string:
	// 	for (int i = 0; i < replacement->GetLength(); i++) {
	// 		data->Insert(loc + i, (*replacement)[i]);}
	// }
	data->Insert(loc,replacement);
}



void do_substitutions(CString *data, CString *subs_filename)
{
	TRY {
		CStdioFile file(*subs_filename, CFile::modeRead);
		while (true) {
			CString buffer;   // holds line from file
			CString pattern;
			CString replacement;
			file.ReadString(buffer);
			// handle end of file
			if (buffer.GetLength() == 0) break;
			if (parse(&buffer, &pattern, &replacement)) {
				substitute(data, &pattern, &replacement);
			} else {
				cout << "Bad pattern/replacement line: " << buffer << endl;
				return;
			}
		}	
	}
	CATCH(CFileException, e ) {
		cout << "File could not be opened or read " << e->m_cause << endl;
	}
	END_CATCH
}


void process_file(CString *filename, CString *subs_filename)
{
	// read in filename to a CString
	TRY {   
		CFile file(*filename, CFile::modeRead);
		int size = file.GetLength();
		// read the data, allocate more than we need
		char *data = new char[size + 16];
		file.Read(data, size);
		// files are not zero-terminated but string should be:
		data[size] = 0;
		// now we can make a CString from the data:
		CString content(data);
		delete data; // data is no longer needed
		do_substitutions(&content, subs_filename);
		// write the data
		file.Close();
		file.Open(*filename, CFile::modeWrite);
		file.Write(content, content.GetLength());
		file.SetLength(content.GetLength());
		file.Close();
	}
	CATCH(CFileException, e ) {
		cout << "File could not be opened or read " << 
			    e->m_cause << " " << *filename << endl;
	}
	END_CATCH

}




int main(int argc, char *argv[])
{
	if (argc < 3) {
		cout << "Not enough input arguments" << endl;
		cout << "Usage: substitute subs-file src1 src2 ..." << endl;
	} else {
		CString subs_filename(argv[1]);
		for (int i = 2; i < argc; i++) {
			CString filename(argv[i]);
			process_file(&filename, &subs_filename);
		}
	}
	return 0;
}


