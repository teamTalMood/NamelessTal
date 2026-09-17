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
class UTextBlock;
class UImage;
class ATalPlayerCharacter;

UCLASS()
class NAMELESSTAL_API UNTPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWithStatComponent(UStatComponent* InStatComponent);

	// 호리병 개수 표시용. 캐릭터의 OnFlaskCountChanged에 바인딩한다
	void InitializeWithPlayerCharacter(ATalPlayerCharacter* InPlayerCharacter);

	void InitializeWithTalComponent(UNTTalComponent* InTalComponent);

	void ShowWidgetMessage(const FText& Title, const FText& Body, float Duration);

	// 사망 시 화면을 검게 덮는다. 부활할 때까지 유지된다
	void ShowDeathScreen();

	// 부활 시 검정을 걷어낸다
	void HideDeathScreen();

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

	// 호리병 아이콘. 개수가 0이 되면 흐리게 만든다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Flask;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_FlaskCount;

	// 사망 시 화면을 덮는 검정. 계층에서 HUD 요소들보다 아래(= 위에 그려지는 쪽)에 둔다
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_DeathFade;

	// 없으면 페이드 없이 즉시 전환된다. 나중에 만들면 자동으로 적용됨
	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> Anim_DeathFade;

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

	UPROPERTY()
	TObjectPtr<ATalPlayerCharacter> PlayerCharacter;

	UFUNCTION()
	void HandleHPChanged(float NewHP, float MaxHP);

	UFUNCTION()
	void HandleSoulChanged(float NewSoul, float MaxSoul);

	UFUNCTION()
	void HandleFlaskCountChanged(int32 NewCount, int32 MaxCount);

	void RefreshAll();

	// 페이드 아웃 끝난뒤 검정을 Collapsed로 되돌리기 위한 타이머
	FTimerHandle DeathFadeTimer;
};
