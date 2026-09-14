// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TalPlayerController.generated.h"


class UNTPlayerHUDWidget;
class UNTMessageWidget;

UENUM(BlueprintType)
enum class ENTMessageStyle : uint8
{
	Notice UMETA(DisplayName = "알림 배너"), Tutorial UMETA(DisplayName = "튜토리얼")
};


UCLASS()
class NAMELESSTAL_API ATalPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<class UInputMappingContext*> DefaultMappingContexts;

	// UI 관련
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNTMessageWidget> NoticeWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNTMessageWidget> TutorialWidgetClass;

	UPROPERTY()
	TObjectPtr<UNTMessageWidget> NoticeWidget;

	UPROPERTY()
	TObjectPtr<UNTMessageWidget> TutorialWidget;


protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNTPlayerHUDWidget> PlayerHUDClass;
	UPROPERTY()
	TObjectPtr<UNTPlayerHUDWidget> PlayerHUD;

public : // UI
	void ShowMessage(ENTMessageStyle Style, const FText& Title, const FText& Body, float Duration);
};
