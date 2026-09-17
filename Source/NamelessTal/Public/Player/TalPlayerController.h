// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TalPlayerController.generated.h"


class UNTPlayerHUDWidget;
class UNTMessageWidget;
class UNTPauseMenuWidget;
class ATalPlayerCharacter;
class UNTAreaNoticeWidget;
class UTexture2D;

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

	// 지역 도착 알림. 트리거 안에 있는 동안 계속 떠 있는다.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNTAreaNoticeWidget> AreaNoticeWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNTMessageWidget> TutorialWidgetClass;

	UPROPERTY()
	TObjectPtr<UNTAreaNoticeWidget> AreaNoticeWidget;

	UPROPERTY()
	TObjectPtr<UNTMessageWidget> TutorialWidget;


protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNTPlayerHUDWidget> PlayerHUDClass;
	UPROPERTY()
	TObjectPtr<UNTPlayerHUDWidget> PlayerHUD;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNTPauseMenuWidget> PauseMenuClass;

	UPROPERTY()
	TObjectPtr<UNTPauseMenuWidget> PauseMenu;

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TObjectPtr<class UInputAction> PauseAction;

	uint8 bISControllerSetted : 1 = false;

public: // UI
	void ShowMessage(ENTMessageStyle Style, const FText& Title, const FText& Body, float Duration);

	// 지역 트리거 진입 시 호출. 나갈 때까지 계속 떠 있는다.
	void ShowAreaNotice(const FText& AreaName, const FText& Description, UTexture2D* Icon);

	// 지역 트리거 이탈 시 호출
	void HideAreaNotice();

	void SetControllerSettings(ATalPlayerCharacter* InPawn = nullptr);

public: // Boss HP
	// GameMode의 OnStartBossGame 델리게이트에 바인딩되는 핸들러
	void HandleStartBossGame();

	// GameMode -> Controller -> HUD -> HPBarWidget 순으로 보스 HP 갱신
	void UpdateBossHP(float CurrentHP, float MaxHP);

	// GameMode -> Controller -> HUD -> HPBarWidget 순으로 보스 넋 갱신
	void UpdateBossSoul(float CurrentSoul, float MaxSoul);

	void TogglePauseMenu();
};
