# ALSV4_CPP_CloneCoding
동영상 링크 : https://www.youtube.com/watch?v=RvJIIwIDNOU

## 개발환경
- ' UE5.4 '
- ' Jetbrain Rider '
- ' DotNet SDK version 6.0.302 '

## 주요기능
- ' ALSV4 C++ Clone Coding '
- ' ALSV4 locomotion UE5_Mannequin 리타겟팅 '
- ...Climbing, Swimming, Overlay 등등 차후 커스텀해갈 예정

## 기억에 남는 버그들

ALS 애니메이션들을 UE5_Mannequin에 리타겟팅을 하는 과정에서 애님모디파이, 커브, IK들를 디테일하게 다루는 ALS라서 크래쉬도 자주나고 캐릭터가 뒤틀리거나 하는 여러가지 문제들을 겪었다. 오랜시간 헤딩해가며 정보를 모으는 시간이 상당히 길었고 힘들었다.

1. 애니메이션 리타겟팅을 하게 되면 당연스럽게 Additive Animation 설정이 초기값인 'No additive'로 설정되어 있었고 이 애니메이션들로 인하여 캐릭터가 뒤틀리고 괴물처럼 변하였다.
   처음엔 원인을 알지 못해 굉장히 당황했지만 헤딩해가며 Additive Animation 설정이 문제였다는걸 알게 되었다.
2. 맨틀링 애니메이션을 리타겟팅할때 캐릭터가 공중에 붕뜨는 문제가 발생하였다. 유튜브 검색을 통해 열심히 정보를 찾았고,
   IK리타게터에서 체인매핑을 할때 root, hand, leg 이렇게 세개의 본체인을 눌러 Detail 패널에서 트랜슬레이션모드를 Globally Scaled로 설정해주고 다시 에셋을 리타겟팅해주니 문제가 해결되었다.
3. 아무래도 ALS캐릭터 스켈레톤메시와 UE5마네킹에 스켈레톤메시가 다르고 여러 커브값과 IK설정들을 사용하기 때문에 겪는 문제들도 많았다. 스켈레톤메시에 커브들을 추가해주고 가상본을 만들어주고 최대한 ALS 스켈레톤메시과 동일하게 UE5_Mannequin의 스켈레톤메시를 수정해줬다.
   그렇지 않으면, 발을 사정없이 떤다던가 발이 공중에서 파닥거린다거나 하는 잡다한 오류들이 정말 많이 발생하였고 이를 수정하기 위해 하나씩 버그들을 픽스해 나갔다.

## 느낀점

- ALS의 디테일한 움직임은 구현해보고 싶었던 로코모션이였기 때문에 c++샘플이 있는지 찾아보고 코드를 분석하고 클론코딩하였다. ALS는 움직임을 디테일하게 구현하기 위해 커브 값과 IK 설정을 세밀하게 조정해 현실감 있는 움직임을 구현하는 것을 알게 되었다.
  하나하나 기능을 구현해보고 버그를 픽스해가며 ALS에 복잡한 원리를 조금이나마 파악하고 리타겟팅하는 능력 및 여러가지 에디터를 다루는 능력이 성장하게된 계기가 된거같다.
