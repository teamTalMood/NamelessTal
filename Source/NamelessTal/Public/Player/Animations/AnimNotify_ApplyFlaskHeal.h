#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ApplyFlaskHeal.generated.h"

/**
 * 호리병 몽타주에서 실제로 약을 삼키는 시점에 찍는 노티파이.
 * 캐릭터의 ApplyFlaskHeal()을 불러 개수 차감과 HP 회복을 수행한다.
 */
UCLASS()
class NAMELESSTAL_API UAnimNotify_ApplyFlaskHeal : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};