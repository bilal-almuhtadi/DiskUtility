/*
\file:		bce.cpp
\author:	Bilal Al-Muhtadi
\date:		Apr 17, 2020
\version:	1.0
\brief:		a program that will that shows disk usage (based on the UNIX command).
*/

#include<locale>
#include<iostream>
#include<fstream>
#include <Windows.h>
#include <filesystem>
#include <string>
#include <deque>
#include <sstream>

using namespace std::filesystem;

//Function declarations
std::vector<std::pair<std::string, uintmax_t>> scan(path const& f, double clusterSize, std::vector<std::pair<std::string, uintmax_t>>& v);
uintmax_t rscan(path const& f, double clusterSize);
bool sort_size(std::pair<std::string, uintmax_t>& lhs, std::pair<std::string, uintmax_t>& rhs);
std::string convertToHumanReadable(uintmax_t x);
void checkSwitches(std::string switches, bool& dublicatesS, bool& dublicatesK, bool& dublicatesH, bool& dublicatesZ, bool& dublicatesN, bool& dublicatesR, bool& dublicatesB, bool& clusterSizeReset, bool& summary, bool& clusterSize1000, bool& humanReadable, bool& sortBySize, bool& sortByName, bool& reverse, bool& outputInBytes, bool& bh, bool& kblock, bool& nz);
void displayOutput(path const& folderPath, std::vector<std::pair<std::string, uintmax_t>> dataV, std::vector<std::pair<std::string, uintmax_t>> specFolderV, double clusterSize, bool& summary, bool& clusterSize1000, bool& humanReadable, bool& sortBySize, bool& sortByName, bool& reverse, bool& outputInBytes, bool& specificFolder);

int main(int argc, char* argv[])
{
	//populate a deque with the args
	std::pmr::deque<std::string> argsD(&argv[1], &argv[argc]);

	double clusterSize = 4096.00;
	//path folderPath = "C:\\Temp";
	path folderPath = current_path();

	std::vector<std::pair<std::string, uintmax_t>> dataV;
	std::vector<std::pair<std::string, uintmax_t>> specFolderV;
	std::vector<path> pathsV;

	//c. Declare some variables to represent input options
	bool specificFolder = false;
	bool summary = false;
	bool clusterSize1000 = false;
	bool humanReadable = false;
	bool reverse = false;
	bool sortBySize = false;
	bool sortByName = false;
	bool outputInBytes = false;
	bool switchesBool = false;
	bool clusterSizeReset = false;

	//errors switches
	bool dublicatesS = false;
	bool dublicatesK = false;
	bool dublicatesH = false;
	bool dublicatesZ = false;
	bool dublicatesN = false;
	bool dublicatesR = false;
	bool dublicatesB = false;
	bool bh = false;
	bool nz = false;
	bool kblock = false;
	bool validVal = true;
	

	std::string switches;
	std::string availablSwitches = "skhznrb";

	//check if there is no arg passed then we will run the programa with the default values (block-size=4096, path = the current path)
	if (argsD.empty())
	{
		dataV = scan(folderPath, clusterSize, specFolderV);

		for (size_t i = 0 ; i < dataV.size() ; i++)
		{
			std::cout << std::left <<std::setw(8) << dataV[i].second << dataV[i].first << std::endl;
		}

		return EXIT_SUCCESS;

	}

	//if we pass args in commandline interface
	if (argsD.size() > 0)
	{
		
		path specifiedPath;

		for (auto i = argsD.begin(); i != argsD.end(); i++)
		{
			if (i->at(0) != '-')
			{
				specifiedPath = *i;

				//check if the path is healthy
				if (exists(specifiedPath))
				{
					//push it back to a vector that will hold the multi passed paths
					pathsV.push_back(specifiedPath);
					specificFolder = true;
				}
				else
				{
					std::cout << "Error: folder <" << specifiedPath.string() << "> doesn't exist.";
					return EXIT_FAILURE;
				}
			}
		}


		//had to use another loop same as the above one because for block-size i wanted to erase it after I use it so i need to break the loop 
		for (auto i = argsD.begin(); i != argsD.end(); i++) 
		{
			
			if (i->find("--block-size=") != std::string::npos)
			{
				std::string valueStr = i->substr(13);
				for (size_t i = 0; i < valueStr.length(); i++)
				{
					if (isdigit(valueStr[i]))
						continue;
					else
					{
						validVal = false;
						break;
					}
				}

				if (validVal)
				{
					 clusterSizeReset = true;
					std::stringstream ss1(valueStr);
					double valueNum;
					ss1 >> valueNum;

					clusterSize = valueNum;
				}
				else
				{
					std::cout << "Error: block-size value is invalid <" << valueStr << ">" << std::endl;
					return EXIT_FAILURE;
				}

				argsD.erase(i);
				break;
			}
		}

		//checking here again if its empty in case we only pass 1 arg which is --block-size
		if (argsD.empty())
		{
			dataV = scan(folderPath, clusterSize,  specFolderV);

			for (size_t i = 0; i < dataV.size(); i++)
			{
				std::cout << std::left << std::setw(8) << dataV[i].second << " " << dataV[i].first << std::endl;
			}

			return EXIT_SUCCESS;

		}

		std::string helpStr = "du (c) 2020, Bilal Al-Muhtadi\n===========================================================\nVersion 1.1.0\nA disk usage utility inspired by the UNIX du command.\n\nUsage: du [-skhb] [--help] [--version] [--block-size=dddd] [folder]*\n\nExamples:\n  du\n    > display the sum of the cluster sizes of each directory\n      starting the cwd\n\n  du folder\n    > display the sum of the cluster sizes of each directory\n      starting with 'folder'\n\n  du -h\n    > display the results in a human readable form\n\n  du -s\n    > display only the final summary\n\n  du -b\n    > display in bytes\n\n  du -k\n    > cluster size is 1024\n\n  du -z\n    > display the list sorted by size\n\n  du -n \n    > display the list sorted by name\n\n  du -r\n    > display the list in reverse order\n\n  du --block-size=dddd\n    > set the cluster size to the specified integer > 0\n\n  du --help \n    > displays the help \n\n  du --version \n    > displays version number in the format d.d.d";
		
		//check for some conditions
		for (auto& data : argsD)
		{

			if (data == "--version")
			{
				std::cout << "1.1.0" << std::endl;
				return EXIT_SUCCESS;
			}

			if (data == "--help")
			{
				std::cout << helpStr << std::endl;
				return EXIT_SUCCESS;
			}

			//check if there is switches also check if the switches are available
			if (data[0] == '-')
			{
				
				if (data[1] != '-')
				{
					switchesBool = true;

					switches = argsD.front().substr(1);

					for (size_t i = 0; i < switches.length(); i++)
					{

						auto it = availablSwitches.find(switches[i]); 

						if (it == std::string::npos)
						{
							std::cout << "Error: unknown switches: <" << switches[i] << ">" << std::endl;
							return EXIT_FAILURE;
						}
					}
				}
			}
		}//end for each

		
		//if no specific folder was passed
		if (!specificFolder)
		{
			if (switchesBool)
			{
				//call the methods to check the switches
				checkSwitches(switches, dublicatesS,dublicatesK, dublicatesH, dublicatesZ, dublicatesN, dublicatesR, dublicatesB, clusterSizeReset, summary, clusterSize1000, humanReadable, sortBySize, sortByName, reverse, outputInBytes,  bh, kblock, nz);

				//check for errors
				if (bh)
				{
					std::cout << "Error: cannot use both -b and -h\n";
					return EXIT_FAILURE;
				}

				if (nz)
				{
					std::cout << "Error: -n and -z switches are incompatible.";
					return EXIT_FAILURE;
				}

				if (kblock)
				{
					std::cout << "Error: -k and --block-size are incompatible.";
					return EXIT_FAILURE;
				}

				if (dublicatesS)
				{
					std::cout << "Error: duplicate switches: <s>";
					return EXIT_FAILURE;
				}

				if (dublicatesK)
				{
					std::cout << "Error: duplicate switches: <k>";
					return EXIT_FAILURE;
				}

				if (dublicatesH)
				{
					std::cout << "Error: duplicate switches: <h>";
					return EXIT_FAILURE;
				}

				if (dublicatesZ)
				{
					std::cout << "Error: duplicate switches: <z>";
					return EXIT_FAILURE;
				}

				if (dublicatesN)
				{
					std::cout << "Error: duplicate switches: <n>";
					return EXIT_FAILURE;
				}

				if (dublicatesR)
				{
					std::cout << "Error: duplicate switches: <r>";
					return EXIT_FAILURE;
				}

				if (dublicatesB)
				{
					std::cout << "Error: -k and --block-size are incompatible.";
					return EXIT_FAILURE;
				}

				dataV = scan(folderPath, clusterSize, specFolderV);

				displayOutput(folderPath, dataV, specFolderV, clusterSize, summary, clusterSize1000, humanReadable, sortBySize, sortByName, reverse, outputInBytes, specificFolder);

			}//end switches if
		}
		else
		{
		//here I will use a for each loop for a the vector that holds the multiple folders paths
			for (auto& x : pathsV)
			{
				//set the path to the new one
				folderPath = x;

				//if I have switches arg then go inside this if
				if (switchesBool)
				{

					checkSwitches(switches, dublicatesS, dublicatesK, dublicatesH, dublicatesZ, dublicatesN, dublicatesR, dublicatesB, clusterSizeReset, summary, clusterSize1000, humanReadable, sortBySize, sortByName, reverse, outputInBytes, bh, kblock, nz);

					if (bh)
					{
						std::cout << "Error: cannot use both -b and -h\n";
						return EXIT_FAILURE;
					}

					if (nz)
					{
						std::cout << "Error: -n and -z switches are incompatible.";
						return EXIT_FAILURE;
					}

					if (kblock)
					{
						std::cout << "Error: -k and --block-size are incompatible.";
						return EXIT_FAILURE;
					}

					if (dublicatesS)
					{
						std::cout << "Error: duplicate switches: <s>";
						return EXIT_FAILURE;
					}

					if (dublicatesK)
					{
						std::cout << "Error: duplicate switches: <k>";
						return EXIT_FAILURE;
					}

					if (dublicatesH)
					{
						std::cout << "Error: duplicate switches: <h>";
						return EXIT_FAILURE;
					}

					if (dublicatesZ)
					{
						std::cout << "Error: duplicate switches: <z>";
						return EXIT_FAILURE;
					}

					if (dublicatesN)
					{
						std::cout << "Error: duplicate switches: <n>";
						return EXIT_FAILURE;
					}

					if (dublicatesR)
					{
						std::cout << "Error: duplicate switches: <r>";
						return EXIT_FAILURE;
					}

					if (dublicatesB)
					{
						std::cout << "Error: -k and --block-size are incompatible.";
						return EXIT_FAILURE;
					}

					dataV = scan(folderPath, clusterSize, specFolderV);

					displayOutput(folderPath, dataV, specFolderV, clusterSize, summary, clusterSize1000, humanReadable, sortBySize, sortByName, reverse, outputInBytes, specificFolder);

				}//end switches if
				else//if there is no switches
				{
					if (!specFolderV.empty())
					{
						specFolderV.clear();
					}

					dataV = scan(folderPath, clusterSize, specFolderV);

					for (size_t i = 0; i < specFolderV.size(); i++)
					{
						std::cout << std::left << std::setw(8) << specFolderV[i].second << " " << specFolderV[i].first << std::endl;
					}

				}



			}//end outer for
		}
	}//end argsD if

}//end main

	/*
	\fn:		scan()
	\brief:		This function will can a single folder, using <filesystem> operations to locate subfolders or files in the folder
	\param:		f: the path u want to search, clusterSize: the cluster size u want to use to find size in clusters, v: a vector of pairs of type <string, uintmax_t> that will be used to save some data that will be used for the output
	\return:	a vector of pairs of type <string, uintmax_t> that will hold the name and the size of  
	*/
	std::vector<std::pair<std::string, uintmax_t>> scan(path const& f, double clusterSize, std::vector<std::pair<std::string, uintmax_t>>& v)
	{
		directory_iterator dir(f);	//first entry of folder f
		directory_iterator end;		//virtual match to the end of any folder

		std::uintmax_t size;
		uintmax_t sizeInClusters = 0;
		std::uintmax_t totalSizeClusters = 0;
		std::string pathStr;
		std::vector<std::pair<std::string, uintmax_t>> data;
	
		while (dir != end)
		{

			if (is_directory(dir->status()))
			{
				sizeInClusters = rscan(dir->path(), clusterSize);//call rscan() that will scan the sub-folder recursivly 
				pathStr = ".\\" + dir->path().filename().string();
			
				data.push_back(std::make_pair(pathStr, sizeInClusters));
				v.push_back(std::make_pair(dir->path().string(), sizeInClusters));
	
			}
			else if (is_regular_file(dir->status()))
			{
			
				size = std::filesystem::file_size(dir->path().string());

				sizeInClusters = (uintmax_t)ceil(size / clusterSize);
		
			}

			totalSizeClusters += sizeInClusters;
			++dir;
		}
	
		data.push_back(std::make_pair(".", totalSizeClusters));
		v.push_back(std::make_pair(f.string(), totalSizeClusters));
	
		return data;
	}


/*
\fn:		rscan()
\brief:		This function will recursively scan the passed path and go inside all subfolders
\param:		f: the path u want to search, clusterSize: the cluster size u want to use to find size in clusters
\return:	a uintmax_t number that represent the total siz in clusters
*/
uintmax_t rscan(path const& f, double clusterSize)
{
	recursive_directory_iterator dir(f);	//first entry of folder f
	recursive_directory_iterator end;		//virtual match to the end of any folder
	
	std::uintmax_t size;
	uintmax_t sizeInClusters = 0;
	std::uintmax_t totalSizeClusters = 0;


	while (dir != end)
	{

		if (is_regular_file(dir->status()))
		{
			//get the file size in bytes, calculate the cluster
			size = std::filesystem::file_size(dir->path().string());

			sizeInClusters = (uintmax_t)ceil(size / clusterSize);

			totalSizeClusters += sizeInClusters;

		}
		++dir;
	}
	return totalSizeClusters;
}

/*
\fn:		sort_size()
\brief:		a predicate function that will be used to sort my <string,uintmax_t> pairs based on the uintmax_t
\param:		two <string,uintmax_t> pairs that u want to compare
\return:	a vector of type string that will contain the whole book code in it
*/
bool sort_size(std::pair<std::string, uintmax_t>& lhs, std::pair<std::string, uintmax_t>& rhs)
{
	return lhs.second < rhs.second;
}


/*
\fn:		convertToHumanReadable()
\brief:		This function will convert size from bytes to human readable value and return it as string
\param:		x: uintmax_t number that represents the size in the bytes
\return:	a string that represent the human readable formula 
*/
std::string convertToHumanReadable(uintmax_t x)
{

	std::vector<std::string> unitsV = { "", "K", "M", "G", "T" };
	int indexV = 0;
	double divisionValue = (double)x;
	while (indexV < 5)
	{
		indexV++;
		divisionValue /= 1024.00;
		if (divisionValue < 1024)
			break;

	}

	if (divisionValue < 10)
	{
		divisionValue *= 10;
		divisionValue += 0.5;
		divisionValue /= 10;
		return (std::to_string(divisionValue).substr(0,3) + unitsV[indexV]);
	}
	else
	{
		divisionValue = round(divisionValue);
		return (std::to_string((int)divisionValue) + unitsV[indexV]);
	}

	
}


/*
\fn:		checkSwitches()
\brief:		This function will turn on the switches based on the user switches entry
\param:		string that represent the switches the user entered and booleans that represnt the switches
\return:	nothing
*/
void checkSwitches(std::string switches, bool& dublicatesS, bool& dublicatesK, bool& dublicatesH, bool& dublicatesZ, bool& dublicatesN, bool& dublicatesR, bool& dublicatesB, bool& clusterSizeReset, bool& summary, bool& clusterSize1000, bool& humanReadable, bool& sortBySize, bool& sortByName, bool& reverse, bool& outputInBytes, bool& bh, bool& kblock, bool& nz)
{

	size_t index = switches.find('s');
	if (index != std::string::npos)
	{
		switches.erase(index, 1);
		
		index = switches.find('s');
		if (index != std::string::npos)
		{
			dublicatesS = true;
		}
		
		summary = true;
	}

	index = switches.find('k');
	if (index != std::string::npos)
	{
		if (clusterSizeReset)
		{
			kblock = true;
		}

		switches.erase(index, 1);
		index = switches.find('k');
		if (index != std::string::npos)
		{
			dublicatesK = true;
		}

		clusterSize1000 = true;
	}

	index = switches.find('h');
	if (index != std::string::npos)
	{
		switches.erase(index, 1);
		index = switches.find('h');
		if (index != std::string::npos)
		{
			dublicatesH = true;
		}

		index = switches.find('b');
		if (index != std::string::npos)
		{
			bh = true;
		}

		humanReadable = true;
	}

	index = switches.find('z');
	if (index != std::string::npos)
	{
		if (sortByName)
		{
			nz = true;
		}

		switches.erase(index, 1);
		index = switches.find('z');
		if (index != std::string::npos)
		{
			dublicatesZ = true;
		}

		sortBySize = true;
	}

	index = switches.find('n');
	if (index != std::string::npos)
	{
		if (sortBySize)
		{
			nz = true;
		}

		switches.erase(index, 1);
		index = switches.find('n');
		if (index != std::string::npos)
		{
			dublicatesN = true;
		}

		sortByName = true;
	}

	index = switches.find('r');
	if (index != std::string::npos)
	{
		switches.erase(index, 1);
		index = switches.find('r');
		if (index != std::string::npos)
		{
			dublicatesR = true;
		}

		reverse = true;
	}

	index = switches.find('b');
	if (index != std::string::npos)
	{
		switches.erase(index, 1);
		index = switches.find('b');
		if (index != std::string::npos)
		{
			dublicatesB = true;
		}

		index = switches.find('h');
		if (index != std::string::npos)
		{
			bh = true;
		}

		outputInBytes = true;
	}
}


/*
\fn:		displayOutput()
\brief:		This function will manipulate the data we gather from scanning the folders to show it based on the switcches that was entered
\param:		a path for the folder that being scanned and the vectors that was used to collect the data and some boolean switches
\return:	nothing
*/
void displayOutput(path const& folderPath, std::vector<std::pair<std::string, uintmax_t>> dataV, std::vector<std::pair<std::string, uintmax_t>> specFolderV, double clusterSize, bool& summary, bool& clusterSize1000, bool& humanReadable, bool& sortBySize, bool& sortByName, bool& reverse, bool& outputInBytes, bool& specificFolder)
{
	bool reported = false;

	if (outputInBytes)
	{
		for (auto& x : dataV)
		{
			x.second *= (uintmax_t)clusterSize;
		}
	}

	if (clusterSize1000)
	{
		clusterSize = 1024;

		dataV = scan(folderPath, clusterSize,  specFolderV);
	}

	if (sortByName)
	{
		std::sort(dataV.begin(), dataV.end());
		std::sort(specFolderV.begin(), specFolderV.end());
	}

	if (sortBySize)
	{
		std::sort(dataV.begin(), dataV.end(), sort_size);
		std::sort(specFolderV.begin(), specFolderV.end(), sort_size);
	}

	if (reverse)
	{
		std::reverse(dataV.begin(), dataV.end());
		std::reverse(specFolderV.begin(), specFolderV.end());
	}



	if (!humanReadable)
	{
		if (summary)
		{
			//dataV = scan(folderPath, clusterSize, specificFolder, specFolderV);
			auto rit = dataV.rbegin();
			auto specRit = specFolderV.rbegin();
			if (!specificFolder)
			{
				std::cout << std::left << std::setw(8) << rit->second << " " << rit->first << std::endl;
			}
			else
			{
				std::cout << std::left << std::setw(8) << specRit->second << " " << specRit->first << std::endl;
			}
			reported = true;
		}

		if (!reported)
		{

			if (!specificFolder)
			{
				for (size_t i = 0; i < dataV.size(); i++)
				{
					std::cout << std::left << std::setw(8) << dataV[i].second << " " << dataV[i].first << std::endl;
				}
			}
			else
			{
				for (size_t i = 0; i < specFolderV.size(); i++)
				{
					std::cout << std::left << std::setw(8) << specFolderV[i].second << " " << specFolderV[i].first << std::endl;
				}
			}
		}
	}

	if (humanReadable)
	{

		std::vector<std::pair<std::string, std::string>> dataHV;
		std::string humanReadableValueStr;

		if (summary)
		{
			//dataV = scan(folderPath, clusterSize, specificFolder, specFolderV);
			auto rit = dataV.rbegin();
			auto specRit = specFolderV.rbegin();
			if (!specificFolder)
			{
				std::cout << std::left << std::setw(8) << rit->second << " " << rit->first << std::endl;
			}
			else
			{
				std::cout << std::left << std::setw(8) << specRit->second << " " << specRit->first << std::endl;
			}
			reported = true;
		}

		if (!reported)
		{
			if (!dataHV.empty())
			{
				dataHV.clear();
			}

			if (!specFolderV.empty())
			{
				specFolderV.clear();
			}

			dataV = scan(folderPath, clusterSize, specFolderV);

			if (sortByName)
			{
				std::sort(dataV.begin(), dataV.end());
				std::sort(specFolderV.begin(), specFolderV.end());
			}

			if (sortBySize)
			{
				std::sort(dataV.begin(), dataV.end(), sort_size);
				std::sort(specFolderV.begin(), specFolderV.end(), sort_size);
			}

			if (reverse)
			{
				std::reverse(dataV.begin(), dataV.end());
				std::reverse(specFolderV.begin(), specFolderV.end());
			}

			if (specificFolder)
			{
				for (auto& x : specFolderV)
				{
					x.second *= (uintmax_t)clusterSize;
				}

				for (auto x : specFolderV)
				{
					//calculations here
					humanReadableValueStr = convertToHumanReadable(x.second);
					dataHV.push_back(make_pair(x.first, humanReadableValueStr));
				}
			}
			else
			{
				for (auto& x : dataV)
				{
					x.second *= (uintmax_t)clusterSize;
				}
				for (auto x : dataV)
				{
					//calculations here
					humanReadableValueStr = convertToHumanReadable(x.second);
					dataHV.push_back(make_pair(x.first, humanReadableValueStr));
				}
			}
			


			for (size_t i = 0; i < dataHV.size(); i++)
			{
				std::cout << std::left << std::setw(7) << dataHV[i].second << dataHV[i].first << std::endl;
			}
		}

	}//end human readable
}