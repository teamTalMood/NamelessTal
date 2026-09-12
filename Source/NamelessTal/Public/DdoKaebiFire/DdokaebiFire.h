// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DdokaebiFire.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBonfireRested);

// 전방선언
class USphereComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UInputAction;


UCLASS()
class NAMELESSTAL_API ADdokaebiFire : public AActor
{
	GENERATED_BODY()
	

public:	
	// Sets default values for this actor's properties
	ADdokaebiFire();

	UPROPERTY(BlueprintAssignable, Category= "Bonfire")
	FOnBonfireRested OnBonfireRested;

	void TriggerRest(AActor* User);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	//불모양 Mesh 넣을 부분
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", 
		meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* FireMesh;

	// Collision 겹침 감지 용도 (상호 작용 가능 거리)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
		meta = (AllowPrivateAccess = "true"))
	USphereComponent* FireInteractionRange;

	//  전용 E키 입력 (화톳불) 
	UPROPERTY(EditAnywhere, Category = "Interact")
	UInputMappingContext* InteractMappingContext;

	UPROPERTY(EditAnywhere, Category = "Interact")
	UInputAction* InteractAction;

	// 화톳불안에 누가 있는지 기억하는 변수
	AActor* OverlappingPlayer = nullptr;

	// Range안에 누군가 들어왔을경우 호출
	UFUNCTION()
	void OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	// Range 밖으로 누군가 나갔을때 호출되는 함수 
	UFUNCTION()
	void OnRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// E키가 눌릴 경우 실행될 함수 
	void OnInteract();
	

};
