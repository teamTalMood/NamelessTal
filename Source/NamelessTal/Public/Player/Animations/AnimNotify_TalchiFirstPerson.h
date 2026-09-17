#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TalchiFirstPerson.generated.h"

UCLASS(meta = (DisplayName = "Talchi First Person"))
class NAMELESSTAL_API UAnimNotify_TalchiFirstPerson : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
