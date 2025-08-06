// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryItemDefinition.h"
#include "UObject/Object.h"
#include "InvSys_InventoryItemInstance.generated.h"

class UInvSys_InventoryFragment_Container;
struct FInvSys_ContainerEntry;
class UInvSys_BaseInventoryObject;
class UInvSys_InventoryComponent;
class UInvSys_InventoryItemFragment;
class UGridInvSys_InventoryItemDefinition;

UENUM()
enum class EInvSys_ReplicateState : uint8
{
	None = 0,
	PreRemove,
	PostAdd,
	PostChange
};

USTRUCT()
struct FInvSys_ItemInstancePropertyHandle
{
	GENERATED_BODY()

	TFunction<void()> OnRepCallback;
};

USTRUCT(BlueprintType)
struct FInvSys_DragItemInstanceMessage
{
	GENERATED_BODY()

	//UInvSys_InventoryComponent

	UPROPERTY(BlueprintReadOnly)
	UInvSys_InventoryItemInstance* ItemInstance = nullptr;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDraggingItem = false;
};

UCLASS(BlueprintType)
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemInstance : public UObject
{
	GENERATED_BODY()

	friend struct FInvSys_ContainerList;
	friend struct FInvSys_ContainerEntry;
	friend class UInvSys_InventoryComponent;

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
	 * 在交换物品至其他库存组件后，会优先调用该函数然后再次执行 构造函数，故通过该函数可以得到 上次的库存组件
	 * 注意：需要使用 DuplicateObject 才能生效！！！
	 * @param DupParams 
	 */
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	virtual void PostRepNotifies() override;

	virtual void PreReplicatedRemove();
	virtual void PostReplicatedAdd();
	virtual void PostReplicatedChange();

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

	void MarkItemInstanceDirty();

protected:
	void BroadcastAddItemInstanceMessage();
	void BroadcastRemoveItemInstanceMessage();
	
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

	void SetItemUniqueID(FGuid Guid);
	
	void SetSlotTag(FGameplayTag Tag);

	void SetIsDraggingItem(bool NewDragState);

	bool IsDraggingItemInstance() const;

	TSubclassOf<UInvSys_InventoryItemDefinition> GetItemDefinition() const
	{
		return ItemDefinition;
	}

	template<class T = UInvSys_InventoryComponent>
	T* GetInventoryComponent() const
	{
		return (T*)InventoryComponent;
	}

	template<class T = UInvSys_InventoryComponent>
	T* GetLastInventoryComponent() const
	{
		return (T*)LastInventoryComponent;
	}

	const FGuid& GetItemUniqueID() const
	{
		return ItemUniqueID;
	}

	const FGameplayTag& GetSlotTag() const
	{
		check(SlotTag.IsValid())
		return SlotTag;
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
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory Item Instance")
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDefinition = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory Item Instance")
	FGuid ItemUniqueID = FGuid();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory Item Instance")
	FGameplayTag  SlotTag;

	UPROPERTY(ReplicatedUsing = OnRep_IsDragging, BlueprintReadOnly, Category = "Inventory Item Instance")
	bool bIsDragging = false;
	UFUNCTION()
	void OnRep_IsDragging();

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Instance")
	TObjectPtr<UInvSys_InventoryComponent> InventoryComponent = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Instance")
	TObjectPtr<UInvSys_InventoryComponent> LastInventoryComponent = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Instance")
	TObjectPtr<AActor> Owner;

	TArray<FInvSys_ItemInstancePropertyHandle> RegisterPropertyArrays;

	bool bIsReadyReplicatedProperties = false;

private:
	TWeakObjectPtr<UInvSys_InventoryFragment_Container> Container_Private = nullptr;
	FInvSys_ContainerEntry* Entry_Private = nullptr;
	EInvSys_ReplicateState ReplicateState = EInvSys_ReplicateState::None;
};
