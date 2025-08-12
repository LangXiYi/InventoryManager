// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Fragment/InvSys_BaseInventoryFragment.h"
#include "InvSys_BaseInventoryObject.generated.h"

class UInvSys_InventoryObjectContent;
class UInvSys_InventoryItemDefinition;
class UInvSys_InventoryItemInstance;
class UInvSys_EquipSlotWidget;

UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_BaseInventoryObject : public UObject
{
	GENERATED_BODY()

	friend class UInvSys_InventoryComponent;
	friend class UInvSys_PreEditInventoryObject;
	friend class UInvSys_InventoryObjectContent;

public:
	UInvSys_BaseInventoryObject();

	/** Fragments 的 RepNotify 优先级大于当前对象，故客户端需要延迟最少一帧，确保该对象的 RepNotify 执行完成。 */
	virtual void PostRepNotifies() override;

	// [Only Server] 在构建库存对象后调用
	// InvComp::ConstructInventoryObject ---> UInvSys_PreEditInventoryObject::NewInventoryObject ---> This Func
	virtual void ConstructInventoryFragment(const TArray<UInvSys_BaseInventoryFragment*>& Fragments);

	// [Server & Client] 在服务器创建库存对象后由库存组件的 OnRep_InventoryObjectList 调用
	virtual void InitInventoryObject(UInvSys_InventoryObjectContent* InventoryObjectContent);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory Object")
	void RefreshInventoryObject();

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void RefreshInventoryFragment(TSubclassOf<UInvSys_BaseInventoryFragment> OutClass);

	/** Only Server todo:: wait edit. */
	void AddInventoryFragment(UInvSys_BaseInventoryFragment* NewFragment);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (DeterminesOutputType = OutClass))
	UInvSys_BaseInventoryFragment* FindInventoryFragment(TSubclassOf<UInvSys_BaseInventoryFragment> OutClass);

	template<class FragmentType>
	FragmentType* FindInventoryFragment()
	{
		for (UInvSys_BaseInventoryFragment* Fragment : InventoryObjectFragments)
		{
			check(Fragment)
			if (Fragment && Fragment->IsA<FragmentType>())
			{
				return (FragmentType*)Fragment;
			}
		}
		return nullptr;
	}

public:
	/**
	 * Getter or Setter
	 */

	FORCEINLINE FGameplayTag GetInventoryObjectTag() const;

	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const;

	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE ENetMode GetNetMode() const;

	FORCEINLINE AActor* GetOwner() const;

	FORCEINLINE bool IsReadyForReplication() const;
	
	FORCEINLINE bool IsUsingRegisteredSubObjectList() const;
	
	FORCEINLINE virtual bool IsSupportedForNetworking() const override { return true; }

	/**
	 * 复制子对象列表
	 * 允许库存对象复制其他子对象至库存组件
	 */
	virtual bool ReplicateSubobjects(UActorChannel *Channel, FOutBunch *Bunch, FReplicationFlags *RepFlags);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Inventory Object")
	FGameplayTag InventoryTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Inventory Object")
	TArray<UInvSys_BaseInventoryFragment*> InventoryObjectFragments;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Object", meta = (AllowPrivateAccess))
	UInvSys_InventoryComponent* InventoryComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Object", meta = (AllowPrivateAccess))
	AActor* Owner_Private;
};

