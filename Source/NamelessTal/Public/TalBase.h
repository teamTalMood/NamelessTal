// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TalBase.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UAnimMontage;//몽타주

UENUM(BlueprintType)
enum class ETalStyle :uint8//나중에 강공격을 위해 남겨두었음
{
	Default		UMETA(DisplayName = "Default"),
	SWORD		UMETA(DisplayName = "SWORD"),
	AXE			UMETA(DisplayName = "AXE"),
	WAND		UMETA(DisplayName = "WAND")
};

UCLASS()
class NAMELESSTAL_API ATalBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ATalBase();

	ETalStyle	  GetTalStyle	  () const;
	UAnimMontage* GetAttackMontage() const;

protected:
	//탈 루트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tal")
	TObjectPtr<USceneComponent> TalRoot;

	//탈 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tal")
	TObjectPtr<UStaticMeshComponent> TalMesh;

	//탈 애니메이션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tal|Animation")
	ETalStyle TalStyle = ETalStyle::Default;

	//기본 공격 애니메이션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tal|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;
};
