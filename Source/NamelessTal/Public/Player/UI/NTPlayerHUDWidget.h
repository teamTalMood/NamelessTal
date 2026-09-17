// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTPlayerHUDWidget.generated.h"

class UProgressBar;
class UStatComponent;
class UNTTalComponent;
class UNTTalIconWidget;
class UNTHPBarWidget;

UCLASS()
class NAMELESSTAL_API UNTPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWithStatComponent(UStatComponent* InStatComponent);

	void InitializeWithTalComponent(UNTTalComponent* InTalComponent);

	void ShowWidgetMessage(const FText& Title, const FText& Body, float Duration);

	// 게임 시작(NativeConstruct) 시 호출. PB_Boss_HP를 찾아 BossHPBarWidget 초기화, Collapsed로 숨겨둠
	void InitBossHPBar();

	// 보스방 입장 시퀀스 종료(OnStartBossGame) 시 호출. BossHPBarWidget을 보이게 전환
	void ShowBossHPBar();

	// 게임모드로부터 전달받은 보스 HP를 BossHPBarWidget에 반영
	void UpdateBossHPBar(float CurrentHP, float MaxHP);

	// 게임모드로부터 전달받은 보스 넋을 BossHPBarWidget에 반영
	void UpdateBossSoulBar(float CurrentSoul, float MaxSoul);

protected:
	virtual void NativeConstruct() override;

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> PB_HP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> PB_Soul;

	UPROPERTY()
	TObjectPtr<class UNTMessageWidget> WDG_Message;

	// 탈 갈아 끼우기 용
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNTTalIconWidget> TalIconWidget;

	// 보스 HP 바. WBP_PlayerHUD에 PB_Boss_HP라는 이름으로 미리 추가되어 있으며, InitBossHPBar 호출 시점에 찾아옴
	UPROPERTY()
	TObjectPtr<UNTHPBarWidget> BossHPBarWidget;

private:
	UPROPERTY()
	TObjectPtr<UStatComponent> StatComponent;

	UFUNCTION()
	void HandleHPChanged(float NewHP, float MaxHP);

	UFUNCTION()
	void HandleSoulChanged(float NewSoul, float MaxSoul);

	void RefreshAll();
};
