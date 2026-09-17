// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TalPlayerController.generated.h"


class UNTPlayerHUDWidget;
class UNTMessageWidget;
class ATalPlayerCharacter;

UENUM(BlueprintType)
enum class ENTMessageStyle : uint8
{
	Notice UMETA(DisplayName = "�˸� ���"), Tutorial UMETA(DisplayName = "Ʃ�丮��")
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

	// UI ����
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

	uint8 bISControllerSetted : 1 = false;

public : // UI
	void ShowMessage(ENTMessageStyle Style, const FText& Title, const FText& Body, float Duration);

	void SetControllerSettings(ATalPlayerCharacter* InPawn = nullptr);
};
