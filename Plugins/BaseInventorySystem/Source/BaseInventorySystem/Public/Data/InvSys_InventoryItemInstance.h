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

/**
 * 针对 FastArray 存储的对象内的成员属性进行包裹
 * todo::简化判断逻辑，使得只需要在类中对需要的属性包裹该模板，即可实现对 OnRep 的监听，并在 PostReplication 中正确执行
 * TInvSys_FastArrayItemProperty<CustomType> Property_A;
 * ...
 * Property_A->RegisterListenItemPropertyChanged([this](){
 *		Execute_CustomFunc();
 * });
 * ...
 */
DECLARE_DELEGATE(FOnListenItemPropertyChanged);
template<class PropertyType>
struct TInvSys_FastArrayItemProperty
{
public:
	TInvSys_FastArrayItemProperty(PropertyType* Value)
	:bIsReadyToNotify(false)
	{
		Property = Value;
	}
	TInvSys_FastArrayItemProperty()
	:Property(nullptr), bIsReadyToNotify(false)
	{
	}

	PropertyType* Get()
	{
		ensureMsgf(Property, TEXT("Tried to access null pointer!"));
		return Property;
	}

	PropertyType& GetRef()
	{
		return *Property;
	}

	bool IsValid() const
	{
		return Property != nullptr && IsValid(Property);
	}

	bool IsReady() const
	{
		return bIsReadyToNotify;
	}

	TInvSys_FastArrayItemProperty<PropertyType>& operator=(TInvSys_FastArrayItemProperty<PropertyType>& NewValue)
	{
		ensureMsgf(NewValue, TEXT("Tried to assign a null pointer to a TInvSys_FastArrayItemProperty!"));
		Property = NewValue.Property;
		return *this;
	}

	FORCEINLINE PropertyType& operator*() const { return *Get(); }
	FORCEINLINE PropertyType* operator->() const { return Get(); }

protected:
	PropertyType* Property;

	bool bIsReadyToNotify;
};

template <typename T>
FORCEINLINE FArchive& operator<<(FArchive& Ar, TInvSys_FastArrayItemProperty<T>& InProperty)
{
	return Ar << InProperty.Property;
}

template <typename T>
FORCEINLINE void operator<<(FStructuredArchiveSlot Slot, TInvSys_FastArrayItemProperty<T>& InProperty)
{
	Slot << InProperty.Property;
}

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
	 * 在执行 DuplicateObject 后获取的 InventoryComponent 是拷贝前的值，故需要在拷贝完成后更新为最新值 
	 * 注意：客户端不会执行该函数，只会重新一遍执行构造函数。
	 */
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	/**
	 * 由于 FastArray 定义的操作函数都是在对象属性复制之前执行
	 * 通过标记以及该函数可以将 FastArray 的操作函数转移至本对象，并将操作时机转移到属性复制完成之后
	 */
	virtual void PostRepNotifies() override;

	// Custom FastArrayItem API Begin -----
	virtual void PreReplicatedRemove();
	virtual void PostReplicatedAdd();
	virtual void PostReplicatedChange();
	// Custom FastArrayItem API End -----

	/**
	 * 如果在 AddItemDefinition/ItemInstance 时传入了特定类型的属性
	 * 那么你就必须在你的类中定义一个与该属性类型一致的 InitItemInstanceProps 函数。
	 * 注意：多个相同类型的该函数，只会调用第一个函数
	 */
	// void InitItemInstanceProps(const int32& Data) {}

	/**
	 * 对于所有需要在 OnRep 函数中执行的逻辑都推荐转移至该函数！
	 * 这是为了避免执行客户端执行顺序与服务器执行顺序不一致所做出的妥协
	 */
	virtual void ReplicatedProperties();

	virtual void RemoveFromInventory();

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

	EInvSys_ReplicateState GetReplicateState() const
	{
		return ReplicateState;
	}

	bool HasAuthority() const;

	ENetMode GetNetMode() const;

	FORCEINLINE bool GetIsReadyReplicatedProperties() const{ return bIsReadyReplicatedProperties; }

	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual FInvSys_ItemInstancePropertyHandle RegisterPropertyListener(const TFunction<void()>& ExecuteFunc)
	{
		FInvSys_ItemInstancePropertyHandle& PropertyHandle = RegisterPropertyArrays.AddDefaulted_GetRef();
		PropertyHandle.OnRepCallback = ExecuteFunc;
		return PropertyHandle;
	}

public:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Instance")
	FOnDragItemInstance OnDragItemInstance;
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
	FInvSys_ContainerEntry* Entry_Private = nullptr; // 其在FastArray中的值
	EInvSys_ReplicateState ReplicateState = EInvSys_ReplicateState::None; // 操作标记，供客户端使用
};
