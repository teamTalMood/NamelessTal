#include "Level/NTAreaTrigger.h"

#include "Components/BoxComponent.h"
#include "Player/TalPlayerCharacter.h"
#include "Player/TalPlayerController.h"

ANTAreaTrigger::ANTAreaTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	// 지역 알림은 넓게 잡는다. 인스턴스마다 조절 가능
	TriggerBox->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ANTAreaTrigger::OnAreaBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ANTAreaTrigger::OnAreaEndOverlap);
}

void ANTAreaTrigger::OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATalPlayerCharacter* Player = Cast<ATalPlayerCharacter>(OtherActor);
	if (Player == nullptr)
	{
		return;
	}

	ATalPlayerController* PlayerController = Cast<ATalPlayerController>(Player->GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	PlayerController->ShowAreaNotice(AreaName, Description, Icon);
}

void ANTAreaTrigger::OnAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATalPlayerCharacter* Player = Cast<ATalPlayerCharacter>(OtherActor);
	if (Player == nullptr)
	{
		return;
	}

	ATalPlayerController* PlayerController = Cast<ATalPlayerController>(Player->GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	PlayerController->HideAreaNotice();
}