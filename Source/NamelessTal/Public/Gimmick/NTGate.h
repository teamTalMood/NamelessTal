// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NTGate.generated.h"

class UPrimitiveComponent;
class UAnimMontage;

UCLASS()
class NAMELESSTAL_API ANTGate : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANTGate();

	// E키 상호작용 시 ATalPlayerCharacter::Interact()의 라인트레이스에서 호출됨
	void RequestOpen(AActor* Player);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<class UBoxComponent> BoxCollisionFrontComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<class UBoxComponent> BoxCollisionBackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<class UStaticMeshComponent> GateFrameMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<class USceneComponent> GateHingeLComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<class USceneComponent> GateHingeRComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<class UStaticMeshComponent> GateMeshLComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<class UStaticMeshComponent> GateMeshRComponent;

	// 상호작용 시 플레이어가 도착해야 할 위치/방향 (모션워핑 타겟) - GateFrameMeshComponent의 소켓에 부착
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<class USceneComponent> PlayerInteractPointComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gate")
	uint8 bGateOpenFront : 1;

	UFUNCTION()
	void OnBoxCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, Category = "Gate|Door")
	TObjectPtr<UAnimMontage> GateOpenMontage;

	UPROPERTY(EditAnywhere, Category = "Gate|Door")
	float DoorOpenDuration = 3.0f;

	bool bIsDoorMoving = false;
	float DoorOpenElapsedTime = 0.0f;
	bool bIsDoorOpened = false;

	FRotator GateMeshLStartRotation;
	FRotator GateMeshRStartRotation;
	FRotator GateMeshLTargetRotation;
	FRotator GateMeshRTargetRotation;

};
