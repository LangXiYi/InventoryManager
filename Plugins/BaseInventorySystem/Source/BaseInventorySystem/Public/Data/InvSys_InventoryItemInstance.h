// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryItemDefinition.h"
#include "UObject/Object.h"
#include "InvSys_InventoryItemInstance.generated.h"

struct FInvSys_ContainerEntry;
class UInvSys_BaseInventoryObject;
class UInvSys_InventoryComponent;
class UInvSys_InventoryItemFragment;
class UGridInvSys_InventoryItemDefinition;

USTRUCT()
struct FInvSys_ItemInstancePropertyHandle
{
	GENERATED_BODY()

	TFunction<void()> OnRepCallback;
};

UCLASS(BlueprintType)
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemInstance : public UObject
{
	GENERATED_BODY()

	friend struct FInvSys_ContainerList;

/*
 * 对于所有的属性，如果需要使用 RepNotify 那么就需要在 OnRep 函数中加入该宏，并且将实际处理的逻辑转移到对应的 Execute 函数中
 */
#define ON_REP_PROPERTY(PropertyName)\
{\
	if (Owner && Owner->HasAuthority()) { Execute_##PropertyName(Old##PropertyName); }\
	else\
	{\
		auto Func = [this, Old##PropertyName]()\
		{\
			this->Execute_##PropertyName(Old##PropertyName);\
		};\
		bIsReadyReplicatedProperties = true;\
		RegisterPropertyListener(Func);\
	}\
}

public:
	UInvSys_InventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/**
	 * 如果在 ItemInstance 中定义了一个需要同步的属性，且该属性在 AddItemDefinition 时传入了该类型的属性
	 * 那么你就必须在你的子类中定义一个与该属性类型一致的 InitItemInstanceProps 函数。*/
	void InitItemInstanceProps(const int32& Data) {}

	/**
	 * 对于所有需要在 OnRep 函数中执行的逻辑都推荐转移至该函数！
	 * 这是为了避免执行客户端执行顺序与服务器执行顺序不一致所做出的妥协
	 */
	virtual void ReplicatedProperties();

	FORCEINLINE bool GetIsReadyReplicatedProperties() const{ return bIsReadyReplicatedProperties; }

	virtual void RemoveFromInventory();

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

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

	TSubclassOf<UInvSys_InventoryItemDefinition> GetItemDefinition() const
	{
		return ItemDefinition;
	}

	template<class T = UInvSys_InventoryComponent>
	T* GetInventoryComponent() const
	{
		return (T*)InvComp;
	}

	const FGuid& GetItemUniqueID() const
	{
		return ItemUniqueID;
	}

	FGameplayTag GetSlotTag() const
	{
		check(SlotTag.IsValid())
		return SlotTag;
	}
	
	FGameplayTag GetLastSlotTag() const
	{
		check(LastSlotTag.IsValid())
		return LastSlotTag;
	}

	int32 GetContainerIndex() const
	{
		return Index;
	}

	FInvSys_ContainerEntry& GetContainerEntryRef() const
	{
		return *Entry_Private;
	}

	bool HasAuthority() const;

	ENetMode GetNetMode() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual FInvSys_ItemInstancePropertyHandle RegisterPropertyListener(const TFunction<void()>& ExecuteFunc)
	{
		FInvSys_ItemInstancePropertyHandle& PropertyHandle = RegisterPropertyArrays.AddDefaulted_GetRef();
		PropertyHandle.OnRepCallback = ExecuteFunc;
		return PropertyHandle;
	}

public:
	/**
	 * 供容器使用，如果物品实例是一个容器，那么这个数组就会保存它拥有的所有物品。
	 * 主要是为了在拖拽容器这类对象时，保存其内部储存所有物品，方便在结束拖拽时统一操作其内部物品。
	 */
	// 这个是否会有问题？比如对象被删除？或是内存泄漏旧对象未卸载？
	UPROPERTY()
	TArray<UInvSys_InventoryItemInstance*> MyInstances;

protected:
	UPROPERTY(Replicated, BlueprintReadOnly)
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDefinition = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FGuid ItemUniqueID = FGuid();

	UPROPERTY(ReplicatedUsing = OnRep_SlotTag, BlueprintReadOnly)
	FGameplayTag  SlotTag;
	UFUNCTION()
	void OnRep_SlotTag(const FGameplayTag& OldSlotTag);
	void Execute_SlotTag(const FGameplayTag& OldSlotTag);
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag  LastSlotTag;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Instance")
	TObjectPtr<UInvSys_InventoryComponent> InvComp = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Instance")
	TObjectPtr<AActor> Owner;

	TArray<FInvSys_ItemInstancePropertyHandle> RegisterPropertyArrays;

	bool bIsReadyReplicatedProperties = false;

private:
	// 物品实例在容器中的索引
	int32 Index = INDEX_NONE;
	FInvSys_ContainerEntry* Entry_Private = nullptr;
};
