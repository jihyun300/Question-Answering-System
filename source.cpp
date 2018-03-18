#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <windows.h>
#include <libxml\parser.h>
#include <libxml/xmlmemory.h>
#include <vector>
#include <list>
#include<utility>
#include<unordered_map>
#include<algorithm>
//#include <chrono>
#include <ctime>
#include <curl\curl.h>
#include<rapidjson\document.h>
using namespace rapidjson;
using namespace std;

void parseDoc(const char *docname);
void getSentences(xmlDocPtr doc, xmlNodePtr cur);
void printSentences(bool isfileout);
void getName();
void nomalization(bool isfileout);
void makeTables();
static int writer(char *data, size_t size, size_t nmemb,std::string *writerData);
string mqlResult(string query);
string returnAnswer(string json, string property);
string getProperty(vector<string> candidates);
void totalProcess(string f);

struct wordInfo{
	string word;
	string pos;
	string ner;
	string dependency;
	int dep;
};
vector< wordInfo> sentence;//key는 word::string;
vector< vector<wordInfo> > sentences;// setence 저장
vector<string >getdependent(vector<wordInfo> s,int dep);
string name[1000];
string mid[1000];
unordered_map<string,vector<string>> property_keyTable;
unordered_map<string,vector<string>> question_answerTable;
string QueryFormat[1000][2];
int questionSize=0;
clock_t start_time;
clock_t end_time;
void main(int argc, char* argv[])
{

	ofstream fileOut;
	char inputQuestion[100];

	//질문 리스트 처리
	if (argc == 2)
	{
		string S = "java -cp \"*\" -Xmx1g edu.stanford.nlp.pipeline.StanfordCoreNLP -annotators tokenize,ssplit,pos,lemma,ner,parse -file ";
		S = S + argv[1];
		start_time=clock();
		system(S.c_str());
		string xmlfile=argv[1];
		xmlfile=xmlfile+".xml";

		//	char *docname = "questions.txt.xml";
		parseDoc(xmlfile.c_str());

		printSentences(true);//output.txt 출력
		cout<< "output.txt를 출력하였습니다."<<endl;
		makeTables();
		cout<<"Property Table을 생성하였습니다"<<endl;
		getName();
		nomalization(true);
		cout<<"\nresult.txt를 출력하였습니다"<<endl;
		
		

	}

	else
	{
		//개별 질문 처리
		while (1)
		{

			cout << "Input : ";
			cin.getline(inputQuestion, sizeof(inputQuestion)); //질문 입력
			string s = inputQuestion;
			if (s == "q")
			{
				cout << "종료합니다." << endl;
				break;
			}
			else if(s.find(".txt")!=string::npos)
			{
				string S = "javaw -cp \"*\" -Xmx1g edu.stanford.nlp.pipeline.StanfordCoreNLP -annotators tokenize,ssplit,pos,lemma,ner,parse -file ";
				S = S + s;
				start_time=clock();
				system(S.c_str());
				string xmlfile=s;
				xmlfile=xmlfile+".xml";

		//	char *docname = "questions.txt.xml";
				parseDoc(xmlfile.c_str());

				printSentences(false);//output.txt 출력
				
				makeTables();
				
				getName();
				nomalization(false);
				
				continue;
		
		

			}
			fileOut.open("input.txt", ios::trunc);

			fileOut << inputQuestion; //텍스트 파일 형성
			fileOut.close();
			memset(inputQuestion, '0', sizeof(inputQuestion)); //배열 비우기

			//라이브러리 활용하여, output 내기.

			system("javaw -cp \"*\" -Xmx1g edu.stanford.nlp.pipeline.StanfordCoreNLP -annotators tokenize,ssplit,pos,lemma,ner,parse -file input.txt");
			char *docname = "input.txt.xml";
			parseDoc(docname);
			printSentences(false);//output.txt 출력
			makeTables();
			getName();
			nomalization(false);
			

		}
	}


	//xml읽기 시작
}

void parseDoc(const char *docname){
	xmlDocPtr doc;
	xmlNodePtr cur;

	sentences.clear();
	doc = xmlParseFile(docname);

	if (doc == NULL){
		fprintf(stderr, "Document not parsed successfully. \n");
		return;
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		fprintf(stderr, "empty document\n");
		xmlFreeDoc(doc);
		return;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")){
		fprintf(stderr, "document of the worng type, root node != root");
		xmlFreeDoc(doc);
		return;
	}
	getSentences(doc, cur);//문장의 단어 구성요소 뽑아옴
	xmlFreeDoc(doc);
	return;
}

void getSentences(xmlDocPtr doc, xmlNodePtr cur){

	int count = 0;
	cur = cur->xmlChildrenNode;
	while (cur != NULL){
		//끝까지 순회하며, 필요한 정보만 뽑아오기
		//		cout << count++ << "\t" << cur->line << " " << cur->name << "\t" << cur->type << "\t" << "\n";

		if (!xmlStrcmp(cur->name, (const xmlChar *) "document")){
			cur = cur->xmlChildrenNode;//하위 노드로 내려감
		}


		else if (!xmlStrcmp(cur->name, (const xmlChar *) "sentences")){
			//sentence시작
			xmlNodePtr sentencePtr = cur->children;
			while (sentencePtr != NULL){

				if (!xmlStrcmp(sentencePtr->name, (const xmlChar *) "sentence")){
					xmlNodePtr sentenceChildPtr = sentencePtr->children;

					while (sentenceChildPtr != NULL){

						if (!xmlStrcmp(sentenceChildPtr->name, (const xmlChar *) "tokens")){
							//tokens에서 word, pos, ner을 자료구조에 저장한다
							//token : <tokens> <token><token>... </tokens>
							xmlNodePtr tokenPtr = sentenceChildPtr->children;

							while (tokenPtr != NULL){
								string word, pos, ner;
								//tmp : <token> <tmp><tmp>... </token>
								if (!xmlStrcmp(tokenPtr->name, (const xmlChar *) "text")){
									tokenPtr = tokenPtr->next; continue;
								}
								xmlNodePtr tmpPtr = tokenPtr->children;
								while (tmpPtr != NULL){
									if (!xmlStrcmp(tmpPtr->name, (const xmlChar *) "word")){
										//	cout << tmpPtr->name << "\t" << tmpPtr->children->content << endl;
										word = (char *)(tmpPtr->children->content);
									}
									else if (!xmlStrcmp(tmpPtr->name, (const xmlChar *) "POS")){
										//	cout << tmpPtr->name << "\t" << tmpPtr->children->content << endl;
										pos = (char *)(tmpPtr->children->content);
									}
									else if (!xmlStrcmp(tmpPtr->name, (const xmlChar *) "NER")){
										//	cout << tmpPtr->name << "\t" << tmpPtr->children->content << endl;
										ner = (char *)(tmpPtr->children->content);
									}

									tmpPtr = tmpPtr->next;
								}
								//변수에 저장
								wordInfo tmpWordInfo;
								tmpWordInfo.word = word;
								tmpWordInfo.pos = pos;
								tmpWordInfo.ner = ner;
								tmpWordInfo.dependency = ".";
								tmpWordInfo.dep = -1;
								sentence.push_back(tmpWordInfo);
								//다음토큰
								tokenPtr = tokenPtr->next;
							}
						}
						else if (!xmlStrcmp(sentenceChildPtr->name, (const xmlChar *) "dependencies")){

							//dep체크
							xmlNodePtr depPtr = sentenceChildPtr->children;
							//dep : <dependencies> <dep><dep>... </dependencies>

							while (depPtr != NULL){
								string dependency;
								if (!xmlStrcmp(depPtr->name, (const xmlChar *) "dep")){
									dependency = (char *)(depPtr->properties->children->content);
									string word; //sentence[word]
									int dep,id; //sentence[word].dep
									xmlNodePtr tmpPtr = depPtr->children;
									//tmp : <dep> <tmp><tmp>... </dep>

									while (tmpPtr != NULL){
										if (!xmlStrcmp(tmpPtr->name, (const xmlChar *) "governor")){
											dep = stoi((char*)(tmpPtr->properties->children->content));
											//	cout << "dep : " << dep << " ";
										}
										else if (!xmlStrcmp(tmpPtr->name, (const xmlChar *) "dependent")){
											id = stoi((char*)(tmpPtr->properties->children->content));
											word = (char *)tmpPtr->children->content;
											//	cout << "  " << word << endl;
										}
										tmpPtr = tmpPtr->next;
									}
									//변수에 저장
									/*sentence[word].dep = dep;
									sentence[word].dependency = dependency;*/
									sentence[id - 1].dep = dep;
									sentence[id - 1].dependency = dependency;

								}
								//다음 dep
								depPtr = depPtr->next;
							}
						}

						sentenceChildPtr = sentenceChildPtr->next;
					}
				}
				//하나의 sentence 끝.
				if (sentence.size() > 0){ //받아온 노드가 있으면 저장
					sentences.push_back(sentence);
					sentence.clear();
				}
				sentencePtr = sentencePtr->next;
			}
		}
		cur = cur->next;
	}
	return;

}

void printSentences(bool isfileout){
	ofstream out;
	if(isfileout)
		out.open("ouput.txt");
	
	for (vector<vector<wordInfo>>::iterator it = sentences.begin(); it != sentences.end(); ++it){
		string question, result;
		question += "Question : ";
		result = "WORD\tPOS\tNER\tDependency\tdep\n";
		for (int i = 0; i < it->size(); i++){
			//cout << it->at(i).word << endl;
			question += it->at(i).word + " ";
			result += it->at(i).word + "\t" + it->at(i).pos + "\t" + it->at(i).ner + "\t" + it->at(i).dependency + "\t" + to_string(it->at(i).dep)+ "\n";
		}
		question += "\n\n";
		result += "\n\n\n";
		if(isfileout)	
			out << question << result;

	//	else
	//		cout<< result;

	}
	if(out.is_open())
		out.close();

}

void getName(){
	string subname[5][2]={{"Andrew","Garfield"},{"Laos",},{"Magic","Moonlight"},{"Yonsei",},{"Yankees",}};
	string name_mid[5][2]={{"Andrew Garfield","/m/0fjmbk"},{"Laos","/m/04hhv"},{"Magic in the Moonlight","/m/0y88_95"},{"Yonsei University","/m/03kwls"},{"New York Yankees","/m/0cqt41"}};
	
	string NNP="NNP";
	string NNPS="NNPS";
	int idx=-1;
	for (vector<vector<wordInfo>>::iterator it = sentences.begin(); it != sentences.end(); ++it){
			string question;
			for(int i=0;i<it->size();i++)
				question += it->at(i).word + " ";
		idx++;
		for(int i=0;i<it->size();i++){


			if(NNP.compare(it->at(i).pos)==0 || NNPS.compare(it->at(i).pos)==0 ){

				for(int j=0;j<5;j++)
				{
					for(int k=0;k<2;k++)
					{
						if(subname[j][k].compare(it->at(i).word)==0)
						{
						
							name[idx]=name_mid[j][0];
							mid[idx]=name_mid[j][1];
							break;
						}
					}


				}
			}
		}
		
		
		
	}
	
}
vector<string> keys;
void nomalization(bool isfileout){
	int answerCnt=0;
	int wrongCnt=0;
	ofstream out,resultout;
	 //한 문장의 key words
	if(isfileout)
	{
		out.open("nomalization.txt");
		resultout.open("result.txt");
	}
	int idx=-1;
	for (vector<vector<wordInfo>>::iterator it = sentences.begin(); it != sentences.end(); ++it){
		//if(isfileout)
		//	start_time=clock();
		vector<string> key_candidates;
		string Key,question;
		int nsubj_dep;
		idx++;
		
		for(int i=0;i<it->size();i++){
			question += it->at(i).word + " ";
			transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
		}

		//What && root
		
		if(it->at(0).word.compare("what")==0 && it->at(0).dependency.compare("root")==0)
		{
			Key+="what\n";
			key_candidates.push_back("what");
			for(int i=1;i<it->size();i++){
				//nsubj가져오기+ nsubj가리키는 애 가져오기
				if(it->at(i).dependency.compare("nsubj")==0 || it->at(i).dependency.compare("rcmod")==0){
					if(it->at(i).pos.compare("NNP")!=0 && it->at(i).pos.compare("NNPS")!=0)
					{
					//	transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);

						nsubj_dep=i+1;

						for(int j=1;j<it->size();j++)
						{
							if(it->at(j).dep==nsubj_dep){
								if(it->at(j).pos.compare("NN") ==0|| it->at(j).pos.compare("JJ") ==0 || it->at(j).pos.compare("NNS")==0){
						//			transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
									Key+=it->at(j).word+"\n";
									key_candidates.push_back(it->at(j).word);
								}
							}
						}
					}
				}
				if(it->at(i).pos.compare("NNP")==0 || it->at(i).pos.compare("NNPS")==0)
				{
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);

				}
				/*else if(it->at(i).dependency.compare("rcmod")==0){
				Key+=it->at(i).word+"\n";
				key_candidates.push_back(it->at(i).word);
				}*/

			}

		}
		else if(it->at(0).word.compare("what")==0 && it->at(0).dependency.compare("root")!=0)
		{
			Key+="what\n";
			key_candidates.push_back("what");
			for(int i=1;i<it->size();i++){
				if(it->at(i).dependency.compare("root")==0){
					if(it->at(i).pos.compare("VBZ")!=0){
			//			transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);

					}
					vector<string> s=getdependent(sentences[idx],i+1);
					keys.clear();
					for(int j=0;j<s.size();j++){
				//		transform(s[j].begin(),s[j].end(),s[j].begin(),::tolower);
						Key+=s[j]+"\n";
						key_candidates.push_back(s[j]);

					}
				}
				else if(it->at(i).dependency.compare("nsubj")==0)
				{
					if(it->at(i).pos.compare("NNP")!=0 && it->at(i).pos.compare("NNPS")!=0){
				//		transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);
						vector<string> s=getdependent(sentences[idx],i+1);
						keys.clear();
						for(int j=0;j<s.size();j++){
				//			transform(s[j].begin(),s[j].end(),s[j].begin(),::tolower);
							Key+=s[j]+"\n";
							key_candidates.push_back(s[j]);
						}
					}
					
					
				}
				if(it->at(i).pos.compare("NNP")==0 || it->at(i).pos.compare("NNPS")==0)
				{
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);

				}
			}
		}
		else if(it->at(0).word.compare("when")==0){
			Key+="when\n";
			key_candidates.push_back("when");
			for(int i=1;i<it->size();i++){
				if(it->at(i).dependency.compare("nsubj")==0){
					if(it->at(i).pos.compare("NNP")!=0 && it->at(i).pos.compare("NNPS")!=0){
				//		transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);
						vector<string> s=getdependent(sentences[idx],i+1);
						keys.clear();
						for(int j=0;j<s.size();j++){
					//		transform(s[j].begin(),s[j].end(),s[j].begin(),::tolower);
							Key+=s[j]+"\n";
							key_candidates.push_back(s[j]);

						}

					}	

				}
				else if(it->at(i).dependency.compare("root")==0){
					if(it->at(i).pos.compare("VBZ")!=0 && it->at(i).pos.compare("VBD")!=0){
				//		transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);
						
					}
					vector<string> s=getdependent(sentences[idx],i+1);
						keys.clear();
						for(int j=0;j<s.size();j++){
				//			transform(s[j].begin(),s[j].end(),s[j].begin(),::tolower);
							Key+=s[j]+"\n";
							key_candidates.push_back(s[j]);
						}
					
				}
				else if(it->at(i).dependency.compare("rcmod")==0){
				//	transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);
				}
				if(it->at(i).pos.compare("NNP")==0 || it->at(i).pos.compare("NNPS")==0)
				{
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);

				}
			}

		}
		else if(it->at(0).word.compare("where")==0){
			Key+="where\n";
			key_candidates.push_back("where");
			for(int i=1;i<it->size();i++){
				if(it->at(i).dependency.compare("rcmod")==0 || it->at(i).dependency.compare("amod")==0){
			//		transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);
				}
				else if(it->at(i).dependency.compare("root")==0){

					if(it->at(i).pos.compare("VBZ")!=0 && it->at(i).pos.compare("VBD")!=0){
			//			transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);
						
					}
					vector<string> s=getdependent(sentences[idx],i+1);
						keys.clear();
						for(int j=0;j<s.size();j++){
			//				transform(s[j].begin(),s[j].end(),s[j].begin(),::tolower);
							Key+=s[j]+"\n";
							key_candidates.push_back(s[j]);
						}
				}
				else if(it->at(i).dependency.compare("nsubj")==0){
			//		transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
					if(it->at(i).pos.compare("NNP")!=0 && it->at(i).pos.compare("NNPS")!=0){
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);

					vector<string> s=getdependent(sentences[idx],i+1);
						keys.clear();
						for(int j=0;j<s.size();j++){
			//				transform(s[j].begin(),s[j].end(),s[j].begin(),::tolower);
							Key+=s[j]+"\n";
							key_candidates.push_back(s[j]);
						}
					}
				}
				if(it->at(i).pos.compare("NNP")==0 || it->at(i).pos.compare("NNPS")==0)
				{
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);

				}
			}
		}
		else if(it->at(0).word.compare("how")==0){
			Key+="how\n";
			key_candidates.push_back("how");
			for(int i=1;i<it->size();i++){
				if(it->at(i).dependency.compare("nsubj")==0){
					if(it->at(i).pos.compare("NN")==0 ||it->at(i).pos.compare("NNS")==0 ){
			//			transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);
					}
				}
				else if(it->at(i).dependency.compare("root")==0){
					if(it->at(i).pos.compare("VBZ")!=0 && it->at(i).pos.compare("VBD")!=0 ){
			//			transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);
						vector<string> s=getdependent(sentences[idx],i+1);
						keys.clear();
						for(int j=0;j<s.size();j++){
			//				transform(s[j].begin(),s[j].end(),s[j].begin(),::tolower);
							Key+=s[j]+"\n";
							key_candidates.push_back(s[j]);
						}
					}
					else if(it->at(i).pos.compare("VBZ")==0){
						for(int l=0;l<it->size();l++){
							if(it->at(l).dep==i+1 && it->at(l).pos.compare("JJ")==0){
			//					transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
								Key+=it->at(l).word+"\n";
								key_candidates.push_back(it->at(i).word);
							}
						}
					}
				}
				if(it->at(i).pos.compare("NNP")==0 || it->at(i).pos.compare("NNPS")==0)
				{
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);

				}
			}
		}
		else if(it->at(0).word.compare("which")==0){
			Key+="which\n";
			key_candidates.push_back("which");
			for(int i=1;i<it->size();i++){
				if(it->at(i).dependency.compare("root")==0){
					if(it->at(i).word.compare("is")!=0){
			//			transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);
					}
					

						vector<string> s=getdependent(sentences[idx],i+1);
						keys.clear();
						for(int j=0;j<s.size();j++){
			//				transform(s[j].begin(),s[j].end(),s[j].begin(),::tolower);
							Key+=s[j]+"\n";
							key_candidates.push_back(s[j]);
						}
					
				}
				else if(it->at(i).dependency.compare("nsubjpass")==0 && it->at(i).pos.compare("NNP")!=0){
			//		transform(it->at(i).word.begin(),it->at(i).word.end(),it->at(i).word.begin(),::tolower);
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);
				}
				if(it->at(i).pos.compare("NNP")==0 || it->at(i).pos.compare("NNPS")==0)
				{
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);

				}
			}
		}
		else if(it->at(0).word.compare("is")==0){
			
			for(int i=1;i<it->size();i++){
				if(it->at(i).dependency.compare("root")==0 && it->at(i).pos.compare("NN")==0){
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);
					vector<string> s=getdependent(sentences[idx],i+1);
					keys.clear();
					for(int j=0;j<s.size();j++){

						Key+=s[j]+"\n";
						key_candidates.push_back(s[j]);
					}
				}
				if(it->at(i).pos.compare("NNP")==0 || it->at(i).pos.compare("NNPS")==0)
				{
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);

				}
			}
		}
		else if(it->at(0).word.compare("who")==0){
			Key+="who\n";
			key_candidates.push_back("who");
			for(int i=1;i<it->size();i++){
				if(it->at(i).dependency.compare("root")==0 && it->at(i).pos.compare("WP")!=0){
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);
					vector<string> s=getdependent(sentences[idx],i+1);
					keys.clear();
					for(int j=0;j<s.size();j++){

						Key+=s[j]+"\n";
						key_candidates.push_back(s[j]);
					}

				}
				else if(it->at(i).dependency.compare("nsubj")==0){
					if(it->at(i).pos.compare("NN")==0 || it->at(i).pos.compare("NNS")==0){
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);
						vector<string> s=getdependent(sentences[idx],i+1);
						keys.clear();
						for(int j=0;j<s.size();j++){

							Key+=s[j]+"\n";
							key_candidates.push_back(s[j]);
						}
					}
				}
				else if(it->at(i).dependency.compare("rcmod")==0){
					if(it->at(i).pos.compare("NN")==0 || it->at(i).pos.compare("NNS")==0){
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);
					}

				}
				if(it->at(i).pos.compare("NNP")==0 || it->at(i).pos.compare("NNPS")==0)
				{
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);

				}
			}
		}
		else if(it->at(0).word.compare("do")==0){
			
			for(int i=1;i<it->size();i++){
				int is_if=0;
				if(it->at(i).dependency.compare("dep")==0){
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);
				
						
				}
				else if(it->at(i).dependency.compare("advmod")==0 && it->at(i).pos.compare("WRB")==0){ //How ,...
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);
				
					
				}
				else if(it->at(i).dependency.compare("dobj")==0 && it->at(i).pos.compare("WP")==0){ //What
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);
				
					
				}
				
				else if(it->at(i).dependency.compare("nsubj")==0 && it->at(i).pos.compare("NN")==0){
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);
					is_if=1;
				}
				else if(it->at(i).dependency.compare("ccomp")==0){
					if(it->at(i).pos.compare("VBN")==0 || it->at(i).pos.compare("VBD")==0){
						Key+=it->at(i).word+"\n";
						key_candidates.push_back(it->at(i).word);
						is_if=1;
					}
				}
				else if(it->at(i).dependency.compare("dobj")==0 && it->at(i).pos.compare("NN")==0){
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);
					is_if=1;
				}
				if(is_if==1){
					vector<string> s=getdependent(sentences[idx],i+1);
						keys.clear();
						for(int j=0;j<s.size();j++){

							Key+=s[j]+"\n";
							key_candidates.push_back(s[j]);
						}
				}
				if(it->at(i).pos.compare("NNP")==0 || it->at(i).pos.compare("NNPS")==0)
				{
					Key+=it->at(i).word+"\n";
					key_candidates.push_back(it->at(i).word);

				}
			}
		}
	//	question += "\n\n";
		
	//	Key += "\n\n";
		//이제 Key_candidate에서 실제 property찾아내야함..
	/*	cout<<"Key candidates? :";
		for(int p=0;p<key_candidates.size();p++){
			cout<<key_candidates[p]<<"/";
		}
		cout<<"\n";*/
		string selectedProperty=getProperty(key_candidates);
		
		string fullProperty;
		string answer_property;
		if(selectedProperty.find(':')!=string::npos) //: 찾았을때
		{
			vector<string> internal;
			stringstream ss(selectedProperty);
			string tok;
			while(getline(ss,tok,':'))
			{
				internal.push_back(tok);
			}
			fullProperty="\""+internal[0]+"\":[{";
			answer_property=internal[0];
			if(internal.size()==2)
			{
				fullProperty+="\""+internal[1]+"\":[]}]";
			}
			else if(internal.size()==3)
			{
				fullProperty+="\""+internal[1]+"\":[],\""+internal[2]+"\":[]}]";
			}
		}
		else //:없을땐 바로 selectedProperty사용가능
		{
			fullProperty="\""+selectedProperty+"\":[]";
			answer_property=selectedProperty;
		}
		//string forprint="{\n\""+selectedProperty+"\":null,\n\"name\":\""+name[idx]+"\",\n\"mid\":\""+mid[idx]+"\"\n}";
		QueryFormat[idx][0]=question;
		QueryFormat[idx][1]="{"+fullProperty+",\"name\":\""+name[idx]+"\",\"mid\":\""+mid[idx]+"\"}";
		
	//	chrono::duration<double> response_time=end-start;
		double response_time=(double)(clock()-start_time)/CLOCKS_PER_SEC;
		start_time=clock();
		string resultJSON=mqlResult(QueryFormat[idx][1]);
//		cout<<"resultJSON: "<<resultJSON<<endl;
//		cout<<"answer_property: "<<answer_property<<endl;
		string answer=returnAnswer(resultJSON,answer_property);

	/*	while(resultJSON.find("error")==string::npos) //에러 안날때까지
		{
			Sleep(12000);
			resultJSON=mqlResult(QueryFormat[idx][1]); //기다렸다가 다시
		}*/

/*		*/
		if(isfileout){
			/*if(question_answerTable.find(QueryFormat[idx][0])!=question_answerTable.end()){
				if(question_answerTable[QueryFormat[idx][0]][0]==answer_property && question_answerTable[QueryFormat[idx][0]][1]==mid[idx]){ //정답률 계산
				answerCnt++;
				}
				else{
					cout<<"Wrong Answer:"<<QueryFormat[idx][0];
					wrongCnt++;
				}
			}*/
			
			resultout<<"Question: "<<QueryFormat[idx][0]<<endl;
			resultout<<"Answer: "<<answer<<"\n\n";
			resultout<<"======================================================================="<<endl;
			resultout<<"Response Time: " <<response_time<<"sec\n";
			resultout<<"Freebase Input:\n"<<QueryFormat[idx][1]<<endl;
			resultout<<"Freebase Response:\n";
			resultout<<resultJSON;
			resultout<<"\n=======================================================================\n\n";
		//	Sleep(13000);
		/*	out<<"Keys:\n"<<Key;
			out<<"key_candidates:\n";
			for(int r=0;r<key_candidates.size();r++){
				out<<key_candidates[r]<<endl;
			}*/

		//	out << question << Key<<selectedProperty<<"\n\n";
		}
		else{
		//	cout<<"Question: "<<QueryFormat[idx][0]<<endl;
			cout<<"Answer: "<<answer<<"\n\n";
			cout<<"======================================================================="<<endl;
			cout<<"Response Time: "<< response_time<<"sec\n";
			cout<<"Freebase Input:\n"<<QueryFormat[idx][1]<<endl;
			cout<<"Freebase Response:\n";
			cout<<resultJSON;
			cout<<"\n=======================================================================\n\n";
			
		//	cout<< Key <<selectedProperty;
		}
		

	}
	
	if(out.is_open()){
		out.close();
		
	}
}



vector<string> getdependent(vector<wordInfo> s,int dep){

	for(int i=0;i< s.size();i++){

		if(s.at(i).dep==dep){
			if(s.at(0).word.compare("how")!=0){
				if(s.at(i).pos.compare("NN")==0 || s.at(i).pos.compare("NNS")==0 || s.at(i).pos.compare("JJ")==0){
					keys.push_back(s.at(i).word);


					getdependent(s,i+1);

				}
			}
			else{
				if(s.at(i).pos.compare("NN")==0 || s.at(i).pos.compare("NNS")==0 || s.at(i).pos.compare("IN")==0){
					keys.push_back(s.at(i).word);			
					getdependent(s,i+1);

				}
			}

		}

	}
	return keys;
}


void makeTables(){
	string oneline;
	ifstream infile,answerfile;
	unordered_map<string,vector<string>>::iterator it;
	infile.open("Keywords_new.txt");
	while(!infile.eof()){ //파일 끝까지
		getline(infile,oneline);
		stringstream ss(oneline);
		string tok;
		string prop;
		int i=-1;
		while(getline(ss,tok,'\t')){
			i++;
			transform(tok.begin(),tok.end(),tok.begin(),::tolower);
			if(i==0){		
				prop=tok;
				
			}
			else{			
				property_keyTable[prop].push_back(tok);
			}
		}
		
		
	
	}
	infile.close();
/*	answerfile.open("questions_answers.txt");
	while(!answerfile.eof()){ //파일 끝까지
		getline(answerfile,oneline);
		stringstream ss(oneline);
		string tok;
		string question;
		int i=-1;
		while(getline(ss,tok,':')){
			i++;
			if(i==0){
				question=tok;
			}
			else{
				question_answerTable[question].push_back(tok);
			}
		}
		//cout<<question<<":"<<question_answerTable[question][0]<<":"<<question_answerTable[question][1]<<endl;
	}

	answerfile.close();

/*	
/*	for(it=property_keyTable.begin();it!=property_keyTable.end();it++){
		cout<<"\nKey: "<<it->first;
		cout<<"\nValue: ";
		for(int i=0;i<it->second.size();i++){
			cout<<it->second.at(i)<<" / ";
		}
	}*/



}
string getProperty(vector<string> candidates){
	unordered_map<string,vector<string>>::iterator it;
	int MAX_match_count=-1;
	string MAX_property;
	
	for(it=property_keyTable.begin();it!=property_keyTable.end();it++){ //한 property씩 검사
		int cnt=0;
		for(int i=0;i<candidates.size();i++){ //후보 단어 모두에 대해
			
			auto result=find(begin(it->second),end(it->second),candidates[i]);
			if(result!=end(it->second)){ //매칭이 있으면
				cnt++;
			}
		}
		if(MAX_match_count<cnt){
			MAX_match_count=cnt;
			MAX_property=it->first;
		}
	}
/*	cout<<"count: "<<MAX_match_count<<endl;
	for(int k=0;k<candidates.size();k++){
		cout<<candidates[k]<<"/";
	}*/
	
	return MAX_property;
}

static int writer(char *data, size_t size, size_t nmemb, std::string *writerData){
	if (writerData == NULL)
		return 0;

	writerData->assign(data);
	return size * nmemb;
}

string mqlResult(string query){
   static std::string buffer;
   CURL *curl;
   CURLcode res;
   string url = "https://www.googleapis.com/freebase/v1/mqlread?query=";
   string key = "key=AIzaSyDajgcEAesqVKpiiAHmJbB20bfq5xVFiPA";
   curl = curl_easy_init();
   if(curl) {
      url.append(curl_easy_escape( curl , query.c_str(), 0)); //쿼리를 escape처리 한 후 append함
      url.append("&" + key);
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str() ); //url+query 설정함
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
      /* Perform the request, res will get the return code */ 
      res = curl_easy_perform(curl);

      /* Check for errors */ 
      if(res != CURLE_OK)
         fprintf(stderr, "curl_easy_perform() failed: %s\n",
         curl_easy_strerror(res));

      /* always cleanup */ 
      curl_easy_cleanup(curl);
      buffer.erase (buffer.find_last_of("}")+1, buffer.size()-buffer.find_last_of("}"));
      return buffer;
   }

   else{
      return "curl error!";
   }}
string returnAnswer(string json, string property){
   Document document;
   document.Parse(json.c_str());
   string answer="";

   
   
   if(document.HasMember("error") || document.HasMember("errors")){ //에러 처리
      return "query ERROR!";
   }
   if(document["result"].IsNull() || document["result"][property.c_str()].Size()==0){
      return "result ERROR!";
   }
   //array 안의 object에서 답이 나올 경우
   if(document["result"][property.c_str()].Begin()->IsObject())   {
      for (Value::ConstValueIterator itr = document["result"][property.c_str()].Begin()->MemberBegin()->value.Begin(); itr != document["result"][property.c_str()].Begin()->MemberBegin()->value.End(); ++itr){
         if(itr->IsString()){
            answer.append(itr->GetString());
         }
         else if(itr->IsDouble()){
            answer.append(to_string(itr->GetDouble()));
         }
         else if(itr->IsInt()){
            answer.append(to_string(itr->GetInt()));
         }
       else if (itr->IsInt64()){
          answer.append(to_string(itr->GetInt64()));
       }
       else if (itr->IsUint()){
          answer.append(to_string(itr->GetUint()));
       }
       else if (itr->IsUint64()){
          answer.append(to_string(itr->GetUint64()));
       }
       else if (itr->IsNull()){
          answer.append("null");
       }
       else{
          answer.append("error!");
       }
         answer.append("/");
      }
   }
   else{ //array 안에서 바로 답이 나올 경우
      for (Value::ConstValueIterator itr = document["result"][property.c_str()].Begin(); itr != document["result"][property.c_str()].End(); ++itr){
         if(itr->IsString()){
            answer.append(itr->GetString());
         }
         else if(itr->IsDouble()){
            answer.append(to_string(itr->GetDouble()));
         }
         else if(itr->IsInt()){
            answer.append(to_string(itr->GetInt()));
         }
       else if (itr->IsInt64()){
          answer.append(to_string(itr->GetInt64()));
       }
       else if (itr->IsUint()){
          answer.append(to_string(itr->GetUint()));
       }
       else if (itr->IsUint64()){
          answer.append(to_string(itr->GetUint64()));
       }
       else if (itr->IsNull()){
          answer.append("null");
       }
       else{
          answer.append("error!");
       }
         answer.append("/");
      }
   }

   answer.pop_back();
   return answer;
}