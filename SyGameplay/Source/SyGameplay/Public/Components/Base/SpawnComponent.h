/**
 * @brief 参考自 FlowSolo 项目
 * @see https://github.com/MothCocoon/FlowSolo
 */

#pragma once

#include "Components/ArrowComponent.h"
#include "Engine/StreamableManager.h"

#include "SpawnComponent.generated.h"

class UAnimationAsset;
class UAnimInstance;
class UBehaviorTree;

USTRUCT()
struct FQuestSpawnParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float ActorScale;

	UPROPERTY(EditAnywhere, Category = "Spawn AI")
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, Category = "Spawn AI")
	TSubclassOf<UAnimInstance> AnimInstance;

	UPROPERTY(EditAnywhere, Category = "Spawn AI")
	TObjectPtr<UAnimationAsset> AnimationAsset;

	UPROPERTY(EditAnywhere, Category = "Spawn AI")
	bool bNoCollisionFail;

	FQuestSpawnParams()
		: ActorScale(1.0f)
		, AnimationAsset(nullptr)
		, bNoCollisionFail(true)
	{
	}
};

/**
* Spawn Component
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class SYGAMEPLAY_API USpawnComponent : public UArrowComponent
{
	GENERATED_UCLASS_BODY()

protected:
	TWeakObjectPtr<AActor> SpawnedActor;
	FStreamableManager StreamableManager;
	
public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool Spawn(const FQuestSpawnParams& SpawnParams);
	void Despawn();

private:
	static void SetAnimationOnSpawnedMesh(USkeletalMeshComponent* SkeletalMeshComponent, const FQuestSpawnParams& SpawnParams);

	template<typename T>
	T* LoadAsset(TSoftObjectPtr<UObject> AssetPtr)
	{
		if (AssetPtr.IsNull())
		{
			return nullptr;
		}

		if (AssetPtr.IsPending())
		{
			const FSoftObjectPath& AssetRef = AssetPtr.ToSoftObjectPath();
			AssetPtr = Cast<T>(StreamableManager.LoadSynchronous(AssetRef, false));
		}

		return Cast<T>(AssetPtr.Get());
	}
};
