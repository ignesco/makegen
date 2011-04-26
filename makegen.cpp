/*
* Copyright (c) 1999-2006,2007, Craig S. Harrison
*
* All rights reserved.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/
#define _ASSERTE
#include <stdlib.h>
#include <stdio.h>

#include "cshTypes_Collection.h"
#include "cshTypes_String.h"
#include <string.h>

class CSHTextFileWriter
{
	public:
		CSHTextFileWriter(char *fname):f(NULL),filename(fname)
		{
		}

		virtual ~CSHTextFileWriter()
		{
		}

		virtual void writeLine(char *l)
		{
			fputs(l,f);
			fputs("\n",f);
		}

                virtual void write(char *l)
		{
			fputs(l,f);
		}

		virtual void open()
		{
			f = fopen(filename.GetBuffer(),"wr");
		}

		virtual void close()
		{
			fclose(f);
		}

	private:
		FILE *f;
		CSHString filename;
};

template<class outputClass>
class CSHFileReader
{
	public:
		virtual void process(outputClass *obj,char *fname)
		{
			char temp[1000];
			
			FILE *f = fopen(fname,"rt");

			if(f!=NULL)
			{
				while(fgets(temp,999,f)!=NULL)
				{
					CSHString s = temp;
					obj->processLine(s);
				}

				fclose(f);
			}
			else
			{
				printf("cannot open file %s\n",fname);
				exit(2);
			}
		}
};

struct target
{
	target(char *tname):targetName(tname)
	{
	}

	virtual ~target()
	{
		deleteStringList(sourceFiles);
		deleteStringList(LibraryPath);
		deleteStringList(Libraries);
	}

	static CSHString getRootNameFromString(CSHString *s)
        {
	  CSHString retVal;
	  int pos = s->reverseFindChar('.');
	  if(pos!=-1)
	    retVal = s->extract(0,pos);
	  else
	    retVal = *s;

	  return retVal;
	}

	virtual void deleteStringList(CSHCollection<CSHString *>::collection &slist)
	{
		for(int i=0;i<slist.getNumberOfItems();i++)
			delete slist.getValueAtIndex(i);
	}

        int targetType;
	CSHString targetName;
	CSHCollection<CSHString *>::collection sourceFiles;
	CSHCollection<CSHString *>::collection LibraryPath;
	CSHCollection<CSHString *>::collection Libraries;

        static const int EXE;
        static const int STATIC;
        static const int DYNAMIC;
};

const int target::EXE=0;
const int target::STATIC=1;
const int target::DYNAMIC=2;

class programModel
{
	public:

		programModel()
		{
		}

		virtual ~programModel()
		{
		        int i;
			for(i=0;i<includeDirs.getNumberOfItems();i++)
				delete includeDirs.getValueAtIndex(i);

			for(i=0;i<targets.getNumberOfItems();i++)
				delete targets.getValueAtIndex(i);
		}


                virtual void setTargetType(char *targetName,char *targetType)
                {
		  target *t = findTarget(targetName);

		  if(t!=NULL)
		    {
		      if(strcmp(targetType,"EXE")==0)
			{
			  t->targetType = target::EXE;
			}
		      else if(strcmp(targetType,"STATIC")==0)
			{
			  t->targetType = target::STATIC;
			} 
		      else if(strcmp(targetType,"DYNAMIC")==0)
			{
			  t->targetType = target::DYNAMIC;
			}
		      else
			printf("unknown target type:%s:.\n",targetType);
		    }
		 else
			{
				printf("Could not find target :!%s!\n",targetName);
			} 
		}

		virtual int setFirstTarget(char *firstTargetName)
		{
			int retVal = 1;
			target *t = findTarget(firstTargetName);

			if(t!=NULL)
			{
				firstTarget = firstTargetName;
			}
			else
				retVal = 0;

			return retVal;
		}

		virtual int addTarget(char *targetName)
		{
		        int retVal = 1;
		        target *t = findTarget(targetName);

			if(t==NULL)
			{
				t = new target(targetName);
				targets.add(t);
			}
			else
				retVal = 1;

			return retVal;
		}

		virtual int addSourceFile(char *targetName,char *sourceName)
		{
			int retVal = 1;
			target *t = findTarget(targetName);

			if(t!=NULL)
			{
				if(getIndexOfString(&t->sourceFiles,sourceName)==-1)
					t->sourceFiles.add(new CSHString(sourceName));
				else
					retVal = 0;
			}
			else
				retVal = 0;
			return retVal;
		}

		virtual int addIncludeDir(char *dirName)
		{
			int retVal = 1;
			if(getIndexOfString(&includeDirs,dirName)==-1)
				includeDirs.add(new CSHString(dirName));
			else
				retVal = 0;

			return retVal;
		}

		virtual int addLibrary(char *targetName,char *libraryPath,char *libraryName)
		{
			int retVal = 1;
			target *t = findTarget(targetName);

			if(t!=NULL)
			{
				if(getIndexOfString(&t->LibraryPath,libraryPath)==-1)
					t->LibraryPath.add(new CSHString(libraryPath));

				if(getIndexOfString(&t->Libraries,libraryName)==-1)
					t->Libraries.add(new CSHString(libraryName));
				else
					retVal = 0;
			}
			else
				retVal = 0;

			return retVal;
		}

		CSHString firstTarget;
		CSHCollection<CSHString *>::collection includeDirs;
		CSHCollection<target *>::collection targets;

		virtual target *findTarget(char *tname)
		{
			target *retVal = NULL;
			for(int i=0;i<targets.getNumberOfItems();i++)
			{
				if(targets.getValueAtIndex(i)->targetName.equal(tname))
				{
					retVal = targets.getValueAtIndex(i);
					break;
				}
			}
			
			return retVal;
		}

		virtual int getIndexOfString(CSHCollection<CSHString *>::collection *slist,char *str)
		{
			int retVal = -1;
			for(int i=0;i<slist->getNumberOfItems();i++)
			{
				if(slist->getValueAtIndex(i)->equal(str))
				{
					retVal = i;
					break;
				}
			}

			return retVal;
		}
};

int replaceCharX(CSHString &sourceStr,char what,char withchar)
{
  int retVal = 0;
  int len = sourceStr.GetLength();
  char *buff = sourceStr.GetBuffer();

  for(int i=0;i<len;i++)
    {
      if(buff[i]==what)
	{
	  buff[i] = withchar;
	  retVal++;
	}
    }

  return retVal;
}

int convertCompoundPath(CSHString &sourceStr)
{
  int retVal = replaceCharX(sourceStr,'/','_');

  replaceCharX(sourceStr,'.','X');

  return retVal;
}

class makeFileWriter
{
	public:
		makeFileWriter(programModel *p):pm(p)
		{
		}
		virtual ~makeFileWriter()
		{
		}

		virtual void output(CSHTextFileWriter *tfw)
		{
			tfw->open();
			tfw->writeLine("# (c) CHIME makegen - generated file to regenerate run makegen again.\n");

			//now write the includes.
			
			tfw->write("CFLAGS = -c");
			for(int i=0;i<pm->includeDirs.getNumberOfItems();i++)
			  {
			    tfw->write(" -I");
			    tfw->write(*(pm->includeDirs.getValueAtIndex(i)));
			  }

			tfw->writeLine("");
			tfw->writeLine("");

			tfw->writeLine("############");
			tfw->writeLine("# first_target");
			tfw->writeLine("############");
			tfw->writeLine("");
			tfw->write("first_target: ");
			tfw->writeLine(pm->firstTarget);
			tfw->writeLine("");
			tfw->writeLine("");

			CSHString cleanAll;
			CSHString makeAll;

			for(int i=0;i<pm->targets.getNumberOfItems();i++)
			  {
			    target *currTarget = pm->targets.getValueAtIndex(i);

			    int numOfSrc = currTarget->sourceFiles.getNumberOfItems(); 
			    if(numOfSrc>0)
			      {
				CSHString targetName = currTarget->targetName;
				tfw->writeLine("#########");
				tfw->write("# ");
				tfw->writeLine(targetName);
				tfw->writeLine("#########");
				CSHString targetNameUpper(targetName);
				targetNameUpper.toUpper();

				int simpleSourceCount = 0;
				int compoundSourceCount = 0;
				for(int j=0;j<numOfSrc;j++)
				  {
				    CSHString *sourceFile = currTarget->sourceFiles.getValueAtIndex(j);
				    CSHString temp(*sourceFile);
				    
				    int numOfReplaces = convertCompoundPath(temp);
				    if(numOfReplaces==0)
				      simpleSourceCount++;
				    else
				      compoundSourceCount++;

				  }

				CSHString targetRoot = target::getRootNameFromString(&targetNameUpper);
				CSHString simpleTargetObj(targetRoot);
				simpleTargetObj.Cat("_OBJ");
				
				tfw->write(simpleTargetObj);
				tfw->write(" =");
				int numOfSrc = currTarget->sourceFiles.getNumberOfItems();

				int currentSourceIndex = 0;
				for(int j=0;j<numOfSrc;j++)
				  {
				    CSHString *sourceFile = currTarget->sourceFiles.getValueAtIndex(j);
				    CSHString sourceRoot = target::getRootNameFromString(sourceFile);
				    CSHString simpleSourceRoot(sourceRoot);
				    
				    int numOfReplaces = convertCompoundPath(simpleSourceRoot);

				    if(numOfReplaces==0)
				      {
					currentSourceIndex++;
					//printf("#%s#%s\n",sourceFile->GetBuffer(),sourceRoot.GetBuffer());
					//printf("!");
					
					tfw->write(" ");
					tfw->write(targetName);
					tfw->write("_obj/");
					tfw->write(sourceRoot);
					tfw->write(".o");
					if(simpleSourceCount>1)
					  {
					    if(currentSourceIndex==simpleSourceCount)
					      {
						tfw->writeLine("");
						tfw->writeLine("");
					      }
					    else
					      tfw->writeLine(" \\");
					  }
					else
					  tfw->writeLine("");
				      }
				  }

				/////////////////////////////
				CSHString compoundTargetObj(targetRoot);
				compoundTargetObj.Cat("_EX_OBJ");
				
				tfw->write(compoundTargetObj);
				tfw->write(" =");

				currentSourceIndex = 0;
				for(int j=0;j<numOfSrc;j++)
				  {
				    CSHString *sourceFile = currTarget->sourceFiles.getValueAtIndex(j);
				    CSHString sourceRoot = target::getRootNameFromString(sourceFile);
				    CSHString simpleSourceRoot(sourceRoot);
				    
				    int numOfReplaces = convertCompoundPath(simpleSourceRoot);

				    if(numOfReplaces>0)
				      {
					currentSourceIndex++;
					//printf("#%s#%s\n",sourceFile->GetBuffer(),sourceRoot.GetBuffer());
					//printf("!");
					
					tfw->write(" ");
					tfw->write(targetName);
					tfw->write("_obj/");
					tfw->write(simpleSourceRoot);
					tfw->write(".o");
					if(compoundSourceCount>1)
					  {
					    if(currentSourceIndex==compoundSourceCount)
					      {
						tfw->writeLine("");
						tfw->writeLine("");
					      }
					    else
					      tfw->writeLine(" \\");
					  }
					else
					  tfw->writeLine("");
				      }
				  }

				/////////////////////////////

				tfw->writeLine("");
				tfw->writeLine("");
				
				tfw->write(targetName);
				tfw->write(": $(");
				tfw->write(simpleTargetObj);
				tfw->write(") $(");
				tfw->write(compoundTargetObj);
				tfw->writeLine(")");

				if(currTarget->targetType==target::EXE)
				  {
				    tfw->write("\tcc");
				  }
				else if(currTarget->targetType==target::STATIC)
				  {
				    tfw->write("\tld -Ur -r");
				  }
				else if(currTarget->targetType==target::DYNAMIC)
				  {
				    tfw->write("\tcc -shared");
				  }
				
				tfw->write(" $(");
				tfw->write(simpleTargetObj);
				tfw->write(") $(");
				tfw->write(compoundTargetObj);
				tfw->write(") -o ");
				tfw->write(targetName);
				for(int j=0;j<currTarget->LibraryPath.getNumberOfItems();j++)
				  {
				    tfw->write(" -L");
				    tfw->write(*(currTarget->LibraryPath.getValueAtIndex(j)));
				  }
				for(int j=0;j<currTarget->Libraries.getNumberOfItems();j++)
				  {
				    tfw->write(" -l");
				    tfw->write(*(currTarget->Libraries.getValueAtIndex(j)));
				  }
				
				
				
				tfw->writeLine("");
				tfw->writeLine("");
				tfw->write("$(");
				tfw->write(simpleTargetObj);
				tfw->write("):");
				
				//Now write source files...
				for(int j=0;j<numOfSrc;j++)
				  {
				    CSHString *sourceFile = currTarget->sourceFiles.getValueAtIndex(j);
				    CSHString simpleSourceRoot(*sourceFile);
				    
				    int numOfReplaces = convertCompoundPath(simpleSourceRoot);

				    if(numOfReplaces==0)
				      {
					tfw->write(" ");
					tfw->write(*sourceFile);
				      }
				  }
				
				tfw->writeLine("");
				
				tfw->writeLine("\tcc $(CFLAGS) $(*F).cpp -o $*.o");
				tfw->writeLine("");

				//Now write compound file compile instructions

				for(int j=0;j<numOfSrc;j++)
				  {
				    CSHString *sourceFile = currTarget->sourceFiles.getValueAtIndex(j);
				    CSHString simpleSourceRoot = target::getRootNameFromString(sourceFile);
				    CSHString compoundSourceRoot(simpleSourceRoot);
				    
				    int numOfReplaces = convertCompoundPath(compoundSourceRoot);

				    if(numOfReplaces>0)
				      {				
					tfw->write(targetName);
					tfw->write("_obj/");

					tfw->write(compoundSourceRoot);
					tfw->write(".o : ");
					tfw->write(simpleSourceRoot);
					tfw->writeLine(".cpp");

					tfw->write("\tcc $(CFLAGS) ");
					tfw->write(simpleSourceRoot);
					tfw->write(".cpp -o ");
					tfw->write(targetName);
					tfw->write("_obj/");
					tfw->write(compoundSourceRoot);
					tfw->writeLine(".o");

				      }
				  }


				
				tfw->writeLine("");
				tfw->write(targetName);
				tfw->write("_");
				tfw->writeLine("depends:");
				tfw->write("\tmakedepend $(CFLAGS) ");	
				for(int j=0;j<numOfSrc;j++)
				  {
				    CSHString *sourceFile = currTarget->sourceFiles.getValueAtIndex(j);
				    tfw->write(*sourceFile);
				    tfw->write(" ");
				  }
				tfw->writeLine("");
				tfw->writeLine("");
				
				makeAll.Cat(" ");
				makeAll.Cat(targetName);
				
				tfw->writeLine("");
				
				tfw->write("clean_");
				tfw->write(targetName);
				tfw->writeLine(":");
				tfw->write("\trm -f *~ ");
				tfw->write(targetName);
				tfw->write("_obj/");
				tfw->writeLine("*.o");
				tfw->writeLine("");
				tfw->writeLine("");
				
				cleanAll.Cat(" clean_");
				cleanAll.Cat(targetName);
			      }
			    else
			      tfw->writeLine("No Source files found.");
			  }

			tfw->write("clean_all:");
			tfw->writeLine(cleanAll);
			
			tfw->writeLine("");
			tfw->write("all:");
			tfw->writeLine(makeAll);

			tfw->writeLine("");
			
			tfw->close();
		}

	programModel *pm;
};

class makeTargetFile
{
	public:
		makeTargetFile(programModel *p):pm(p)
		{
		}

		virtual void processLine(CSHString &s)
		{
			if(s[0]!='#' && s[0]!='\n' && s[0]!='\r')
			{
				if(s.find("TARGETNAME:")==0)
				{
					int p1 = s.findChar(':');
					int p2 = s.findChar(':',p1+1);

					if(p1!=-1 && p2!=-1)
					{
						CSHString t = s.extract(p1+1,p2);
						pm->addTarget(t.GetBuffer());
					}
					else
					{
						printf("cannot find fields for line:\n%s\n",s.GetBuffer());
						exit(1);
					}
				}
				else if(s.find("FIRSTTARGET:")==0)
				{
					int p1 = s.findChar(':');
					int p2 = s.findChar(':',p1+1);

					if(p1!=-1 && p2!=-1)
					{
						CSHString t = s.extract(p1+1,p2);
						pm->setFirstTarget(t.GetBuffer());
					}
					else
					{
						printf("cannot find fields for line:\n%s\n",s.GetBuffer());
						exit(1);
					}
				}
				else if(s.find("SOURCEFILE:")==0)
				{
					int p1 = s.findChar(':');
					int p2 = s.findChar(':',p1+1);
					int p3 = s.findChar(':',p2+1);

					if(p1!=-1 && p2!=-1 && p3!=-1)
					{
						CSHString t1 = s.extract(p1+1,p2);
						CSHString t2 = s.extract(p2+1,p3);
						pm->addSourceFile(t1.GetBuffer(),t2.GetBuffer());
					}
					else
					{
						printf("cannot find fields for line:\n%s\n",s.GetBuffer());
						exit(1);
					}
				}else if(s.find("TARGETTYPE:")==0)
				{
					int p1 = s.findChar(':');
					int p2 = s.findChar(':',p1+1);
					int p3 = s.findChar(':',p2+1);

					if(p1!=-1 && p2!=-1 && p3!=-1)
					{
						CSHString t1 = s.extract(p1+1,p2);
						CSHString t2 = s.extract(p2+1,p3);
						pm->setTargetType(t1.GetBuffer(),t2.GetBuffer());
					}
					else
					{
						printf("cannot find fields for line:\n%s\n",s.GetBuffer());
						exit(1);
					}
				}
				else if(s.find("INCLUDEDIR:")==0)
				{
					int p1 = s.findChar(':');
					int p2 = s.findChar(':',p1+1);

					if(p1!=-1 && p2!=-1)
					{
						CSHString t1 = s.extract(p1+1,p2);
						pm->addIncludeDir(t1.GetBuffer());
					}
					else
					{
						printf("cannot find fields for line:\n%s\n",s.GetBuffer());
						exit(1);
					}
				}
				else if(s.find("LIBRARY:")==0)
				{
					int p1 = s.findChar(':');
					int p2 = s.findChar(':',p1+1);
					int p3 = s.findChar(':',p2+1);
					int p4 = s.findChar(':',p3+1);

					if(p1!=-1 && p2!=-1 && p3!=-1 && p4!=-1)
					{
						CSHString t1 = s.extract(p1+1,p2);
						CSHString t2 = s.extract(p2+1,p3);
						CSHString t3 = s.extract(p3+1,p4);
						pm->addLibrary(t1.GetBuffer(),t2.GetBuffer(),t3.GetBuffer());
					}
					else
					{
						printf("cannot find fields for line:\n%s\n",s.GetBuffer());
						exit(1);
					}
				}
			}
		}

		programModel *pm;
};

int main(int argc,char *argv[])
{
  if(argc!=3)
    {
      printf("Usage : XX inputfile outputfile\n");
      return 0;
    }
  char *ifn = argv[1];
  char *ofn = argv[2];


	programModel *pm = new programModel;
	makeTargetFile mtf(pm);
	CSHFileReader<makeTargetFile> fr;
	fr.process(&mtf,ifn);

	makeFileWriter mfw(pm);
	CSHTextFileWriter ctfw(ofn);
	mfw.output(&ctfw);

	delete pm;

	return 0;
}
