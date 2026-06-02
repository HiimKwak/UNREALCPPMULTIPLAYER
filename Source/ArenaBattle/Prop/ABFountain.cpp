// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"

#include "ArenaBattle.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AABFountain::AABFountain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water"));

	RootComponent = Body;
	Water->SetupAttachment(Body);
	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01'"));
	if (BodyMeshRef.Object)
	{
		Body->SetStaticMesh(BodyMeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Fountain_02.SM_Plains_Fountain_02'"));
	if (WaterMeshRef.Object)
	{
		Water->SetStaticMesh(WaterMeshRef.Object);
	}
	
	// 1. 액터의 리플리케이션 속성 켜주기
	bReplicates = true;
	
	NetUpdateFrequency = 1.f; // 네트워크 전송 빈도 1초에 1번
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();
}

void AABFountain::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 3. 네트웍으로 복제할 속성 DOREPLIFETIME 매크로로 명시
	DOREPLIFETIME(AABFountain, ServerRotationYaw);
}

void AABFountain::OnActorChannelOpen(class FInBunch& InBunch, class UNetConnection* Connection)
{
	Super::OnActorChannelOpen(InBunch, Connection);
	
}

void AABFountain::OnRep_ServerRotationYaw()
{
	FRotator NewRotator = RootComponent->GetComponentRotation();
	NewRotator.Yaw = ServerRotationYaw;

	RootComponent->SetWorldRotation(NewRotator);
	
	// 이전 서버의 업데이트로부터 이번 업데이트까지 걸린 시간 저장
	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;

	// 서버로부터 데이터를 받으면 0으로 초기화
	ClientTimeSinceUpdate = 0.0f;
}

// Called every frame
void AABFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		AddActorLocalRotation(FRotator(0.0f, RotationRate * DeltaTime, 0.0f));
		ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
	}
	else
	{
		// 서버로부터 데이터를 받은 후에 경과한 시간 계산
		ClientTimeSinceUpdate += DeltaTime;

		// 너무 작은 시간이 경과했을 때는 의미가 없음
		if (ClientTimeSinceUpdate < KINDA_SMALL_NUMBER)
		{
			return;
		}

		// 다음 네트워크 패킷 전송 때 전달될 회전 값 예측
		const float EstimateRotationYaw
			= ServerRotationYaw + RotationRate * ClientTimeBetweenLastUpdate;

		// 보간할 비율(alpha) 구하기
		const float LerpRatio
			= ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;

		// 보간(Lerp)
		const float ClientNewYaw 
			= FMath::Lerp(ServerRotationYaw, EstimateRotationYaw, LerpRatio);

		// 회전 값 설정 및 적용
		FRotator ClientRotator = RootComponent->GetComponentRotation();
		ClientRotator.Yaw = ClientNewYaw;

		RootComponent->SetWorldRotation(ClientRotator);
	}
}

