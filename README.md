Question-Answering-System
===============
##### Knowledge based QA system using FreeBase, StanfordNLP with 정원태

Knowledge based QA system이란 사용자의 질의를 이해하고, 지식베이스 내의 정보를 이용하여 응답을 제시하는 시스템이다.

본 프로젝트에서는 다양한 질의어의 형태(언어처리)를 정규화된 형태로 변환(질문 정규화 모듈 구현)하고 사용자의 질의에 적합한 답을 제시하는 시스템을 구현한다.

#### Implementation
- Language: C++
- Tool: Visual Studio 2012
- Database: FreeBase ( 5 entities, 15 properties each )
- Library: Stanford CoreNLP, Freebase API, CURL, JSON
 
> *현재 Freebase는 구글에 인수된 상태로 운영을 중단하였다. 따라서, 본 시스템을 구현했을 때와 현재(2018.03) Freebase에서 데이터를 제공하는 방식이 다르지만, 여전히 개발자들을 위해 DB dump와 API를 제공해주고 있다.*
 
### Process
   [1. Select Entities and Properties from Freebase](#1-select-entities-and-properties-from-freebase)
 
   [2. Implement Natural Language Processing Module](#2-implement-natural-language-processing-module)
 
   [3. Implement Question Normalization Module](#3-implement-question-normalization-module)
 
   [4. Load Question Answer](#4-load-question-answer)
 
   [5. Result](#5-result)

****
## Overview
<p align="center">
   <img src="screenshots/QAoverview.png" width="80%"></img>
</p>

## 1. Select Entities and Properties from Freebase

먼저 우리가 질문에 답할 주제(entity-정보의 세계에서 의미있는 하나의 정보 단위)를 선정한다. 여러 타입(인물, 기업, 영화 등)에서 5개의 Entity를 선정하여 FreeBase에서 각 entity의 속성 및 값을 찾는다. 

우리는 영화배우 앤드류 가필드, 국가 라오스, 영화 "Magic in the Moonlight", 연세대학교 그리고 스포츠팀 뉴욕 양키즈를 선택하였고, 각 주제마다 정답을 찾을 15개 이상의 속성값을 찾았다. 

또한, 이후 자연어처리 모듈 구현 과정에서 사용할 [질문set](/questions.txt)을 만들었다.

'''
 예를 들어, 앤드류 가필드의 여자친구가 누구인지를 알고싶다면 다음과 같이 여러 형태로 물어볼 수 있다. 이 모든 경우에도 답할 수 있도록 미리 질문set을 만들어놓아 대응할 수 있도록 구현하였다.
    
    "Who is Andrew Garfield's partner?"

    "Who have been going out with Andrew Garfield?"
    
    "Who have been dating Andrew Garfield?"
'''   

## 2. Implement Natural Language Processing Module

사용자로부터 들어온 자연어 형태의 질문의 의미를 분석하기 위해 단어의 품사를 분석(POS-tagging)하고, 문장내의 개체명을 인식(Named Entity Recognition)하고, 문장 구조를 분석(Parsing)해야 한다.

본 프로젝트에서는 Stanford CoreNLP를 사용하여 진행하였다. Stanford CoreNLP 라이브러리를 사용하면 질문(input)에 대한 결과를 xml 파일(output)로 생성되는데, 라이브러리 libxml2를 사용하여 프로그램 내에서 xml 파일 정보를 로딩한다.

 ### POS-Tagging
 - 품사(Part-of-speech: 각 단어가 속한 유형)
 - 각 단어의 적합한 품사를 부착한다.
 - 아래 표에서 POS부분을 확인하면 된다.
   
 ### NER (Named Entity Recognition)
 - 인명, 지명, 기관명 등과 같은 개체명 인식 및 분류에 적합한 태그를 부착한다. 주로 명사나, 고유 명사를 후보로 한다.
 - 아래 표에서 NER부분을 확인하면 된다.
 
 ### Parsing
 - 문장을 이루고 있는 구성 성분으로 분해하고, 그들 사이의 구조적 관계를 분석하여 문장의 구조를 결정한다.
 - 문장 내의 구성 성분 간의 관계를 이용하여 다양한 활용이 가능하다.
 
 <p align="center">
   <img src="screenshots/QAoverview.png" width="80%"></img>
</p>

## 3. Implement Question Normalization Module

## 4. Load Question Answer

## 5. Result
