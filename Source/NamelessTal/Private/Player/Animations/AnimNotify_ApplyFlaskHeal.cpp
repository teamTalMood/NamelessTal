#include "Player/Animations/AnimNotify_ApplyFlaskHeal.h"
#include "Player/TalPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_ApplyFlaskHeal::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp == nullptr)
	{
		return;
	}

	if (ATalPlayerCharacter* Character = Cast<ATalPlayerCharacter>(MeshComp->GetOwner()))
	{
		Character->ApplyFlaskHeal();
	}
}