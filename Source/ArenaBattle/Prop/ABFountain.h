// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABFountain.generated.h"

UCLASS()
class ARENABATTLE_API AABFountain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AABFountain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 리플리케이션 콜백 함수는 UFUNCTION으로 등록돼야 넘어감
	UFUNCTION()
	void OnRep_ServerRotationYaw();
	UFUNCTION()
	void OnRep_ServerLightColor();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCChangeLightColor(const FLinearColor& NewLightColor);
	
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerRPCChangeLightColor();
	
	UFUNCTION(Client, Unreliable)
	void ClientRPCChangeLightColor(const FLinearColor& NewLightColor);
	
	// 2. 네트웍으로 복제할 액터의 속성 키워드로 지정
	UPROPERTY(ReplicatedUsing = OnRep_ServerRotationYaw)
	float ServerRotationYaw;
	
	float RotationRate = 30.f;
	
	// 서버로부터 패킷을 받은 후에 경과한 시간을 계산하기 위한 변수
	float ClientTimeSinceUpdate = 0.0f;

	// 서버로부터 데이터를 받고 그 다음 데이터를 받았을 때까지 걸린 시간을 기록할 변수
	float ClientTimeBetweenLastUpdate = 0.0f;

	UPROPERTY(ReplicatedUsing=OnRep_ServerLightColor)
	FLinearColor ServerLightColor;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Water;

};
