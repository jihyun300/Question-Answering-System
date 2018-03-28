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
 
> *현재 FreeBase는 구글에 인수된 상태로 운영을 중단하였다. 따라서, 본 시스템을 구현했을 때와 현재(2018.03) freebase에서 데이터를 제공하는 방식이 다르지만, 여전히 개발자들을 위해 DB dump와 API는 제공해주고 있다.*
 
### Process
   [1. Select Entities and Properties from FreeBase](#1-select-entities-and-properties-from-freebase)
 
   [2. Implement Natural Language Processing Module](#2-implement-natural-language-processing-module)
 
   [3. Implement Question Normalization Module](#3-implement-question-normalization-module)
 
   [4. Load Question Answer](#4-load-question-answer)
 
   [5. Result](#5-result)

****
## Overview
<p align="center">
   <img src="screenshots/QAoverview.png" width="80%"></img>
</p>

## 1. Select Entities and Properties from FreeBase

먼저 우리가 질문에 답할 주제(entity-정보의 세계에서 의미있는 하나의 정보 단위)를 선정한다. 여러 타입(인물, 기업, 영화 등)에서 5개의 Entity를 선정하여 FreeBase에서 각 entity의 속성 및 값을 찾는다.

## 2. Implement Natural Language Processing Module

## 3. Implement Question Normalization Module

## 4. Load Question Answer

## 5. Result
