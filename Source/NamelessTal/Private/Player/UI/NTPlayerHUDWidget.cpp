#include "Player/UI/NTPlayerHUDWidget.h"
#include "MCLOG.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Stat/StatComponent.h"
#include "Player/UI/NTMessageWidget.h"
#include "Player/UI/NTTalIconWidget.h"
#include "Player/TalPlayerCharacter.h"
#include "NPC/Components/NTHPBarWidget.h"
#include "TimerManager.h"
#include "Animation/WidgetAnimation.h"

void UNTPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	WDG_Message = Cast<UNTMessageWidget>(GetWidgetFromName(TEXT("WBP_Notice")));
	ensure(WDG_Message);

	// WBP에서 Collapsed로 두는 걸 잊어도 시작할 때 확실히 숨긴다
	if (IsValid(Img_DeathFade))
	{
		Img_DeathFade->SetVisibility(ESlateVisibility::Collapsed);
	}


	InitBossHPBar();
}

void UNTPlayerHUDWidget::InitializeWithStatComponent(UStatComponent* InStatComponent)
{
	if (!IsValid(InStatComponent)) return;

	if (IsValid(StatComponent))
	{
		StatComponent->OnHPChanged.RemoveDynamic(
			this,
			&UNTPlayerHUDWidget::HandleHPChanged
		);

		StatComponent->OnSoulChanged.RemoveDynamic(
			this,
			&UNTPlayerHUDWidget::HandleSoulChanged
		);
	}

	StatComponent = InStatComponent;

	StatComponent->OnHPChanged.AddUniqueDynamic(
		this,
		&UNTPlayerHUDWidget::HandleHPChanged
	);

	StatComponent->OnSoulChanged.AddUniqueDynamic(
		this,
		&UNTPlayerHUDWidget::HandleSoulChanged
	);

	RefreshAll();
}

void UNTPlayerHUDWidget::InitializeWithPlayerCharacter(ATalPlayerCharacter* InPlayerCharacter)
{
	if (!IsValid(InPlayerCharacter)) return;

	if (IsValid(PlayerCharacter))
	{
		PlayerCharacter->OnFlaskCountChanged.RemoveDynamic(
			this,
			&UNTPlayerHUDWidget::HandleFlaskCountChanged
		);
	}

	PlayerCharacter = InPlayerCharacter;

	PlayerCharacter->OnFlaskCountChanged.AddUniqueDynamic(
		this,
		&UNTPlayerHUDWidget::HandleFlaskCountChanged
	);

	// 델리게이트는 값이 바뀔 때만 오므로 현재값을 한 번 직접 읽어온다
	HandleFlaskCountChanged(
		PlayerCharacter->GetFlaskCount(),
		PlayerCharacter->GetFlaskMaxCount()
	);
}

void UNTPlayerHUDWidget::InitializeWithTalComponent(UNTTalComponent* InTalComponent)
{
	if (!IsValid(TalIconWidget)) return;

	TalIconWidget->InitializeWithTalComponent(InTalComponent);
}

void UNTPlayerHUDWidget::ShowWidgetMessage(const FText& Title, const FText& Body, float Duration)
{
	if (WDG_Message == nullptr)
	{
		return;
	}

	WDG_Message->Show(Title, Body, Duration);
}

void UNTPlayerHUDWidget::ShowDeathScreen()
{
	if (!IsValid(Img_DeathFade))
	{
		return;
	}

	// 걷어내는 중이었다면 취소한다
	GetWorld()->GetTimerManager().ClearTimer(DeathFadeTimer);

	Img_DeathFade->SetVisibility(ESlateVisibility::HitTestInvisible);

	if (Anim_DeathFade)
	{
		StopAnimation(Anim_DeathFade);
		PlayAnimation(Anim_DeathFade);
	}
	else
	{
		// 애니메이션이 없으면 즉시 완전한 검정
		Img_DeathFade->SetRenderOpacity(1.0f);
	}
}

void UNTPlayerHUDWidget::HideDeathScreen()
{
	if (!IsValid(Img_DeathFade))
	{
		return;
	}

	if (!Anim_DeathFade)
	{
		Img_DeathFade->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 등장 애니메이션을 역재생해 서서히 걷어낸다
	PlayAnimationReverse(Anim_DeathFade);

	GetWorld()->GetTimerManager().SetTimer(
		DeathFadeTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (IsValid(Img_DeathFade))
				{
					Img_DeathFade->SetVisibility(ESlateVisibility::Collapsed);
				}
			}),
		Anim_DeathFade->GetEndTime(), false);
}

void UNTPlayerHUDWidget::InitBossHPBar()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	BossHPBarWidget = Cast<UNTHPBarWidget>(GetWidgetFromName(TEXT("PB_Boss_HP")));

	if (IsValid(BossHPBarWidget))
	{
		BossHPBarWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		MC_LOG(LogMC, Error, TEXT("No PB_Boss_HP widget found!!"));
	}
}

void UNTPlayerHUDWidget::ShowBossHPBar()
{
	if (!IsValid(BossHPBarWidget)) return;

	MC_LOG(LogMC, Log, TEXT("Begin"));

	BossHPBarWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UNTPlayerHUDWidget::UpdateBossHPBar(float CurrentHP, float MaxHP)
{
	if (!IsValid(BossHPBarWidget)) return;

	MC_LOG(LogMC, Log, TEXT("Begin"));

	BossHPBarWidget->UpdateHpBar(CurrentHP, MaxHP);
}

void UNTPlayerHUDWidget::UpdateBossSoulBar(float CurrentSoul, float MaxSoul)
{
	if (!IsValid(BossHPBarWidget)) return;

	BossHPBarWidget->UpdateSoulBar(CurrentSoul, MaxSoul);
}

void UNTPlayerHUDWidget::HandleHPChanged(float NewHP, float MaxHP)
{
	if (IsValid(PB_HP))
	{
		PB_HP->SetPercent(
			MaxHP > 0.0f ? NewHP / MaxHP : 0.0f
		);
	}
}

void UNTPlayerHUDWidget::HandleSoulChanged(float NewSoul, float MaxSoul)
{
	if (IsValid(PB_Soul))
	{
		PB_Soul->SetPercent(
			NewSoul > 0.0f ? NewSoul / MaxSoul : 0.0f
		);
	}
}

void UNTPlayerHUDWidget::HandleFlaskCountChanged(int32 NewCount, int32 MaxCount)
{
	if (IsValid(Txt_FlaskCount))
	{
		Txt_FlaskCount->SetText(FText::AsNumber(NewCount));
	}

	if (IsValid(Img_Flask))
	{
		// 다 쓰면 아이콘을 흐리게
		const float IconOpacity = (NewCount > 0) ? 1.0f : 0.35f;
		Img_Flask->SetRenderOpacity(IconOpacity);
	}
}

void UNTPlayerHUDWidget::RefreshAll()
{
	if (!IsValid(StatComponent)) return;

	HandleHPChanged(
		StatComponent->GetCurrentHP(),
		StatComponent->GetMaxHP()
	);
	HandleSoulChanged(
		StatComponent->GetCurrentSoul(),
		StatComponent->GetMaxSoul()
	);
}

void UNTPlayerHUDWidget::NativeDestruct()
{
	if (IsValid(StatComponent))
	{
		StatComponent->OnHPChanged.RemoveDynamic(
			this,
			&UNTPlayerHUDWidget::HandleHPChanged
		);
		StatComponent->OnSoulChanged.RemoveDynamic(
			this,
			&UNTPlayerHUDWidget::HandleSoulChanged
		);

		StatComponent = nullptr;
	
	}

	if (IsValid(PlayerCharacter))
	{
		PlayerCharacter->OnFlaskCountChanged.RemoveDynamic(
			this,
			&UNTPlayerHUDWidget::HandleFlaskCountChanged
		);

		PlayerCharacter = nullptr;
	}

	Super::NativeDestruct();
}