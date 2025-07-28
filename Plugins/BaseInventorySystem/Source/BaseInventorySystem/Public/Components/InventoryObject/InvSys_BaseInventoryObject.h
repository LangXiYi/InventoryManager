// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Fragment/InvSys_BaseInventoryFragment.h"
#include "InvSys_BaseInventoryObject.generated.h"

class UInvSys_InventoryItemDefinition;
class UInvSys_InventoryItemInstance;
class UInvSys_EquipSlotWidget;

UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_BaseInventoryObject : public UObject
{
	GENERATED_BODY()

	friend class UInvSys_InventoryComponent;
	friend class UInvSys_PreEditInventoryObject;

public:
	UInvSys_BaseInventoryObject();

	// [Only Server] 在构建库存对象后调用
	// InvComp::ConstructInventoryObject ---> UInvSys_PreEditInventoryObject::NewInventoryObject ---> This Func
	virtual void ConstructInventoryFragment(const TArray<UInvSys_BaseInventoryFragment*>& Fragments);

	// [Server & Client] 在服务器创建库存对象后由库存组件的 OnRep_InventoryObjectList 调用
	virtual void InitInventoryObject(UObject* PreEditPayLoad);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory Object")
	virtual void RefreshInventoryObject();

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void RefreshInventoryFragment(TSubclassOf<UInvSys_BaseInventoryFragment> OutClass);

	/** Only Server todo:: wait edit. */
	void AddInventoryFragment(UInvSys_BaseInventoryFragment* NewFragment);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (DeterminesOutputType = OutClass))
	UInvSys_BaseInventoryFragment* FindInventoryFragment(TSubclassOf<UInvSys_BaseInventoryFragment> OutClass);

	/**
	 * Getter Or Setter
	 **/

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

	UFUNCTION(BlueprintPure)
	FORCEINLINE FGameplayTag GetInventoryObjectTag() const{ return InventoryObjectTag; }

	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const;

	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE ENetMode GetNetMode() const;

	FORCEINLINE AActor* GetOwner() const;

	FORCEINLINE bool IsReadyForReplication() const;
	
	FORCEINLINE bool IsUsingRegisteredSubObjectList();
	
	FORCEINLINE virtual bool IsSupportedForNetworking() const override { return true; }

	/**
	 * 复制子对象列表
	 * 允许库存对象复制其他子对象至库存组件
	 */
	virtual bool ReplicateSubobjects(UActorChannel *Channel, FOutBunch *Bunch, FReplicationFlags *RepFlags);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Object")
	UInvSys_InventoryComponent* InventoryComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Object")
	FGameplayTag InventoryObjectTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Inventory Object")
	TArray<UInvSys_BaseInventoryFragment*> InventoryObjectFragments;

private:
	bool bIsInitInventoryObject = false;

	UPROPERTY()
	AActor* Owner_Private;
};

/**
 * Pre Edit Inventory Object
 */

UCLASS(EditInlineNew, DefaultToInstanced, Blueprintable)
class BASEINVENTORYSYSTEM_API UInvSys_PreEditInventoryObject : public UObject
{
	GENERATED_BODY()

	friend class UInvSys_BaseInventoryObject;

public:
	template<class InventoryObjectType = UInvSys_BaseInventoryObject>
	InventoryObjectType* NewInventoryObject(UInvSys_InventoryComponent* InvComp)
	{
		if (InvComp)
		{
			InventoryObjectType* InvObj = NewObject<InventoryObjectType>(InvComp);
			InvObj->ConstructInventoryFragment(Fragments);
			return InvObj;
		}
		return nullptr;
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Object")
	FGameplayTag InventoryObjectTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Inventory Object")
	TArray<UInvSys_BaseInventoryFragment*> Fragments;
};
