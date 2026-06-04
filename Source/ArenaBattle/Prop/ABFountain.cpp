// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"

#include "ArenaBattle.h"
#include "Components/PointLightComponent.h"
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
	
	SetNetUpdateFrequency(1.0f); // 네트워크 전송 빈도 1초에 1번
	
	SetNetCullDistanceSquared(4000000.0f); // 거리 기반 연관성 판정에 사용할 거리 값 (20미터 제곱)
	
	// NetDormancy = DORM_Initial; // 처음부터 잠들기 설정하면 네트워크 전송(리플리케이션)이 안됨
	// 서버에서 재우는거기 때문에 클라이언트랑 상관없음.
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda(
				[&]() {
					ServerLightColor = FLinearColor(
					FMath::RandRange(0.0f, 1.0f),
					FMath::RandRange(0.0f, 1.0f),
					FMath::RandRange(0.0f, 1.0f),
					1.0f
					);
					
					OnRep_ServerLightColor();
				}
			), 1.0f, true
		);
		FTimerHandle Handle2;
		GetWorld()->GetTimerManager().SetTimer(
			Handle2,
			FTimerDelegate::CreateLambda(
				[&]() {
					// FlushNetDormancy();
				}
			), 5.0f, false
		);
	}
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

bool AABFountain::IsNetRelevantFor(
	const AActor* RealViewer,
	const AActor* ViewTarget,
	const FVector& SrcLocation) const
{
	bool NetRelevantResult = Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);

	// 연관성이 없다고 판단된 경우에는 뷰어의 위치 출력
	if (!NetRelevantResult)
	{
		AB_LOG(
			LogABNetwork,
			Log,
			TEXT("Not Relevant: [%s] %s"),
			*RealViewer->GetName(),
			*SrcLocation.ToString()
		);
	}

	return NetRelevantResult;
}

void AABFountain::PreReplication(
	IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::PreReplication(ChangedPropertyTracker);
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

void AABFountain::OnRep_ServerLightColor()
{
	if (HasAuthority())
	{
		AB_LOG(LogABNetwork, Log, TEXT("ServerLightColor: %s"), *ServerLightColor.ToString());
	}

	// 서버-클라이언트 모두에서 실행
	if (UPointLightComponent* PointLight = GetComponentByClass<UPointLightComponent>())
	{
		PointLight->SetLightColor(ServerLightColor);
	}
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

