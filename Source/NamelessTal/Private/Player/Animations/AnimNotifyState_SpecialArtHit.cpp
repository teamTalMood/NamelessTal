#include "Player/Animations/AnimNotifyState_SpecialArtHit.h"

#include "Components/SkeletalMeshComponent.h"
#include "Player/TalPlayerCharacter.h"

void UAnimNotifyState_SpecialArtHit::NotifyBegin(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp)
    {
        return;
    }

    ATalPlayerCharacter* Player =
        Cast<ATalPlayerCharacter>(MeshComp->GetOwner());

    if (Player)
    {
        Player->BeginSpecialArtHit(ComboCount);
    }
}

void UAnimNotifyState_SpecialArtHit::NotifyEnd(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp)
    {
        return;
    }

    ATalPlayerCharacter* Player =
        Cast<ATalPlayerCharacter>(MeshComp->GetOwner());

    if (Player)
    {
        Player->EndSpecialArtHit(ComboCount);
    }
}