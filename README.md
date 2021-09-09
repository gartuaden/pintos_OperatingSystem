# pintos_OperatingSystem
"Pintos" Operating System, Sogang University, 2019 Fall Season (PRJ1-PRJ3)


## Overview
### Project 1
사용자에 의한 기본 명령에 대해 프로그램의 실행 환경을 구축하는 것이 이번 프로젝트의 목표이다. 개발 전 기존 Pintos에는 System call, System call handler, Argument passing, User stack이 구현되어 있지 않아 Pintos가 사용자 프로그램을 실행할 수 없다.
Argumemnt passing을 통해 사용자 프로그램의 mutiple arguments를 분석하여 메모리에 할당하는 기능과, 유효하지 않은 포인터를 어떻게 대처할지 처리하는 기능과, 사용자의 명령에 따라 각각 기능을 수행하는 System call handler의 기능과, 추가적인 기능(fibonacci, sum of four int)을 추가할 것이다.
### Project 2
Project1에서 구현한 코드를 수정해 파일 시스템 관련 시스템콜을 완성한다. Project1에선 STDIN, STDOUT에 대해서만 구현했지만, 이를 확장한다.
또한 project1에서 busy waiting으로 구현한 임계구역문제를 Lock과 Semaphore로 수정한다. 
### Project 3
Project 1, 2에서 사용자의 명령을 수행하는 대부분의 기능을 구현했다. Pintos의 경우 기본적으로 round-robin 방식으로 scheduling을 실행하는데, 이는 thread의 우선 순위를 반영하지 못한다.
따라서 이번 project 3에서는 thread의 우선순위를 반영하는 process scheduling을 구현한다. 이를 위해 thread와 여러 synchronization 기법이 활용된다.
Semaphore, lock등의 기법을 통해 synchronization을 구현하고, 추가적으로 BSD scheduler을 구현한다. BSD scheduling을 위해선 MLFQ나 MLRQ의 기법이 사용된다.


* 자세한 내용은 첨부된 document 파일들 참조

