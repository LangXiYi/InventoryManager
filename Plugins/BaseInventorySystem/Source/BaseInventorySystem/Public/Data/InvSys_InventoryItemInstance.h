// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryItemDefinition.h"
#include "UObject/Object.h"
#include "InvSys_InventoryItemInstance.generated.h"

class UInvSys_BaseInventoryObject;
class UInvSys_InventoryComponent;
class UInvSys_InventoryItemFragment;
class UGridInvSys_InventoryItemDefinition;



// DECLARE_DELEGATE_OneParam(FOnInventoryStackChange, FInvSys_InventoryStackChangeMessage);

/**
 * 
 */
UCLASS(BlueprintType)
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UInvSys_InventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/**
	 * 如果在 ItemInstance 中定义了一个需要同步的属性，且该属性在 AddItemDefinition 时传入了该类型的属性
	 * 那么你就必须在你的子类中定义一个与该属性类型一致的 InitItemInstanceProps 函数。*/
	void InitItemInstanceProps(const int32& Data) {}

	virtual void RemoveFromInventory();

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	// FORCEINLINE FOnInventoryStackChange& OnInventoryStackChangeDelegate()
	// {
	// 	return OnInventoryStackChange;
	// }

	FORCEINLINE void BroadcastStackChangeMessage(int32 OldCount, int32 NewCount) // 广播堆叠数量变化事件
	{
		// FInvSys_InventoryStackChangeMessage StackChangeMessage;
		// StackChangeMessage.ItemInstance = this;
		// StackChangeMessage.StackCount = NewCount;
		// StackChangeMessage.Delta = NewCount - OldCount;
		//
		// if (OnInventoryStackChange.ExecuteIfBound(StackChangeMessage))
		// {
		// 	//广播物品堆叠数量变化
		// }
	}
	
	/**
	 * Getter or Setter
	 */
public:
	UFUNCTION(BlueprintPure)
	FText GetItemDisplayName() const
	{
		check(ItemDefinition);
		return GetDefault<UInvSys_InventoryItemDefinition>(ItemDefinition)->GetItemDisplayName();
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UInvSys_InventoryItemFragment* FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

	void SetItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> NewItemDef);

	void SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp);

	void SetItemUniqueID(FGuid Guid);
	
	void SetSlotTag(FGameplayTag Tag);

	void SetStackCount(int32 NewStackCount)
	{
		StackCount = NewStackCount;

		UWorld* World = GetWorld();
		if (World && World->GetNetMode() != NM_DedicatedServer)
		{
			OnRep_StackCount();
		}
	}

	TSubclassOf<UInvSys_InventoryItemDefinition> GetItemDefinition() const
	{
		return ItemDefinition;
	}

	template<class T = UInvSys_InventoryComponent>
	T* GetInventoryComponent() const
	{
		return (T*)InvComp;
	}

	template<class T = UInvSys_InventoryComponent>
	T* GetLastInventoryComponent() const
	{
		check(LastInvComp);
		return (T*)LastInvComp;
	}

	const FGuid& GetItemUniqueID() const
	{
		return ItemUniqueID;
	}

	FGameplayTag GetSlotTag() const
	{
		return SlotTag;
	}

	int32 GetStackCount() const
	{
		return StackCount;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:
	/**
	 * 供容器使用，如果物品实例是一个容器，那么这个数组就会保存它拥有的所有物品。
	 * 主要是为了在拖拽容器这类对象时，保存其内部储存所有物品，方便在结束拖拽时统一操作其内部物品。
	 */
	UPROPERTY()
	TArray<UInvSys_InventoryItemInstance*> MyInstances;
	
protected:
	UPROPERTY(Replicated, BlueprintReadOnly)
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDefinition = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly)
	TObjectPtr<UInvSys_InventoryComponent> InvComp = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly)
	TObjectPtr<UInvSys_InventoryComponent> LastInvComp = nullptr;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	FGuid ItemUniqueID = FGuid();

	UPROPERTY(ReplicatedUsing = OnRep_StackCount, BlueprintReadOnly)
	int32 StackCount = 0;
	UFUNCTION()
	void OnRep_StackCount();
	int32 LastStackCount = 0;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FGameplayTag  SlotTag;

private:
	// FOnInventoryStackChange OnInventoryStackChange;
};
