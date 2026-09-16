#include "Player/Animations/AnimNotify_TalchiFirstPerson.h"
#include "Components/SkeletalMeshComponent.h"
#include "Player/TalPlayerCharacter.h"

void UAnimNotify_TalchiFirstPerson::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!IsValid(MeshComp)) return;
	if (ATalPlayerCharacter* Player = Cast<ATalPlayerCharacter>(MeshComp->GetOwner()))
	{
		// Ignore preview/secondary meshes to avoid duplicate or unrelated notifies.
		if (MeshComp == Player->GetMesh()) Player->EnterTalchiFirstPerson();
	}
}
