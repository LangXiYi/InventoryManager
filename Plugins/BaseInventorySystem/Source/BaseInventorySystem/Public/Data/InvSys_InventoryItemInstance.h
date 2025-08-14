// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryItemInstance.generated.h"

class UInvSys_InventoryComponent;
class UInvSys_InventoryItemFragment;
class UInvSys_InventoryItemDefinition;
class UInvSys_InventoryFragment_Container;

UENUM()
enum class EInvSys_ReplicateState : uint8
{
	None = 0,
	PreRemove,
	PostAdd,
	PostChange
};

USTRUCT(BlueprintType)
struct FInvSys_DragItemInstanceMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	class UInvSys_InventoryItemInstance* ItemInstance = nullptr;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDraggingItem = false;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDragItemInstance, bool, bIsDragging);

/**
 * 库存内容项
 * 注意：修改类成员属性后需要调用 MarkItemInstanceDirty 标记修改！！！
 */
UCLASS(BlueprintType)
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemInstance : public UObject
{
	GENERATED_BODY()

	friend struct FInvSys_ContainerList;
	friend struct FInvSys_ContainerEntry;

public:
	UInvSys_InventoryItemInstance();

	/**
	 * 如果在 AddItemDefinition/ItemInstance 时传入了特定类型的属性
	 * 那么你就必须在你的类中定义一个与该属性类型一致的 InitItemInstanceProps 函数。
	 * 注意：多个相同类型的该函数，只会调用第一个函数
	 */
	// void InitItemInstanceProps(const int32& Data, bool bIsBroadcast) {}

	/**
	 * 在执行 DuplicateObject 后获取的 InventoryComponent 是拷贝前的值，故需要在拷贝完成后更新为最新值 
	 * 注意：客户端不会执行该函数，只会重新一遍执行构造函数。
	 */
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	/**
	 * 由于 FastArray 定义的操作函数都是在对象属性复制之前执行
	 * 通过标记以及该函数可以将 FastArray 的操作函数转移至本对象，并将操作时机转移到属性复制完成之后
	 */
	virtual void PostRepNotifies() override;

	/**
	 * 当物品从库存中删除时触发
	 */
	virtual void RemoveFromInventory() {}

	/**
	 * 当物品在从某一容器转移至另一容器内时触发
	 * todo::目前仅在切换装备容器时，内部物品会触发该函数。
	 */
	virtual void OnTransferItems(UInvSys_InventoryFragment_Container* ContainerFragment) {}

protected:
	// Custom FastArrayItem API Begin -----
	virtual void PreReplicatedRemove();
	virtual void PostReplicatedAdd();
	virtual void PostReplicatedChange();
	// Custom FastArrayItem API End -----

public:
	/**
	 * Getter or Setter
	 */

	/** 获取物品的名称 */
	UFUNCTION(BlueprintPure)
	FText GetItemDisplayName() const;

	/** 根据 ItemDefinition 的 CDO 获取指定类型的片段信息 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UInvSys_InventoryItemFragment* FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const;

	/** 根据 ItemDefinition 的 CDO 获取指定类型的片段信息 */
	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

	/** 设置物品的定义类，保存了当前对象所有定义相关信息。 */
	FORCEINLINE void SetItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> NewItemDef)
	{
		ItemDefinition = NewItemDef;
	}

	/** 设置物品的唯一ID，目前并未实际使用该属性。 */
	FORCEINLINE void SetItemUniqueID(FGuid Guid)
	{
		ItemUniqueID = Guid;
	}


	/** 设置物品的库存标签，主要用来表示当前物品在库存组件中的位置。 */
	FORCEINLINE void SetSlotTag(FGameplayTag Tag)
	{
		InventoryObjectTag = Tag;
	}

	/** 设置物品的拖拽状态，同时广播该状态更新。 */
	void SetIsDraggingItem(bool NewDragState);

	/** 判断当前物品是否处在拖拽状态 */
	FORCEINLINE bool IsDraggingItemInstance() const
	{
		return bIsDragging;
	}

	/** 获取物品的定义信息 */
	FORCEINLINE TSubclassOf<UInvSys_InventoryItemDefinition> GetItemDefinition() const
	{
		return ItemDefinition;
	}

	/** 获取物品当前所在的库存组件 */
	template<class T = UInvSys_InventoryComponent>
	FORCEINLINE T* GetInventoryComponent() const
	{
		// check(InventoryComponent);
		return (T*)InventoryComponent;
	}

	/** 获取物品的唯一ID */
	FORCEINLINE const FGuid& GetItemUniqueID() const
	{
		check(ItemUniqueID.IsValid());
		return ItemUniqueID;
	}

	/** 获取物品的库存对象标签 */
	FORCEINLINE const FGameplayTag& GetInventoryObjectTag() const
	{
		check(InventoryObjectTag.IsValid())
		return InventoryObjectTag;
	}

	int32 GetItemRemainStackCount() const;
	int32 GetItemMaxStackCount() const;
	int32 GetItemStackCount() const
	{
		return StackCount;
	}

	FORCEINLINE void SetItemStackCount(int32 NewStackCount)
	{
		StackCount = NewStackCount;
	}
	
	bool HasAuthority() const;

	ENetMode GetNetMode() const;

	FORCEINLINE virtual bool IsSupportedForNetworking() const override { return true; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	void BroadcastAddItemInstanceMessage();
	void BroadcastRemoveItemInstanceMessage();

public:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Instance")
	FOnDragItemInstance OnDragItemInstance;
	/**
	 * 供容器使用，如果物品实例是一个容器，那么这个数组就会保存它拥有的所有物品。
	 * 主要是为了在拖拽容器这类对象时，保存其内部储存所有物品，方便在结束拖拽时统一操作其内部物品。
	 */
	// 这个是否会有问题？比如对象被删除？或是内存泄漏旧对象未卸载？
	UPROPERTY()
	TArray<UInvSys_InventoryItemInstance*> PayloadItems;

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory Item Instance", meta = (ExposeOnSpawn))
	int32 StackCount = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory Item Instance", meta = (ExposeOnSpawn))
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDefinition = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory Item Instance")
	FGuid ItemUniqueID = FGuid();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory Item Instance")
	FGameplayTag InventoryObjectTag;

	UPROPERTY(ReplicatedUsing = OnRep_IsDragging, BlueprintReadOnly, Category = "Inventory Item Instance")
	bool bIsDragging = false;
	UFUNCTION()
	void OnRep_IsDragging();

	/* 根据 FastArray 的处理函数更新该标记，并在 RepNotify 同步完成后根据该标记调用自定义的处理函数 */
	EInvSys_ReplicateState ReplicateState = EInvSys_ReplicateState::None;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Instance", meta = (AllowPrivateAccess))
	TObjectPtr<UInvSys_InventoryComponent> InventoryComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Instance", meta = (AllowPrivateAccess))
	TObjectPtr<AActor> Owner_Private;
};
