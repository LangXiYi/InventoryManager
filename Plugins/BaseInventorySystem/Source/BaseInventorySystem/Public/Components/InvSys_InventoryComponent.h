// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InventoryObject/InvSys_BaseInventoryObject.h"
#include "InventoryObject/Fragment/InvSys_InventoryFragment_Container.h"
#include "InvSys_InventoryComponent.generated.h"

class UInvSys_InventoryFragment_Container;
class AInvSys_PickableItems;
class UInvSys_InventoryLayoutWidget;
class UInvSys_InventoryItemDefinition;
class UInvSys_PreEditInventoryObject;
class UInvSys_BaseInventoryObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDropItemInstanceToWorld, AInvSys_PickableItems*, DropItem);

UCLASS(Abstract, BlueprintType, meta=(BlueprintSpawnableComponent))
class BASEINVENTORYSYSTEM_API UInvSys_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UInvSys_InventoryControllerComponent;

public:
	// Sets default values for this component's properties
	UInvSys_InventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 构建库存对象列表，推荐在BeginPlay阶段就调用该函数 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	virtual void ConstructInventoryObjects();

	/** 仅本地控制器，由用户手动调用。===> CreateDisplayWidget */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory Component")
	virtual UInvSys_InventoryLayoutWidget* CreateDisplayWidget(APlayerController* NewPlayerController);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	void EquipItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, FGameplayTag SlotTag);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	bool EquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	bool UnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	/**
	 * 支持自定义参数传入物品实例，支持在不同库存组件下转移物品，在转移时需要注意将旧物品的实例删除。
	 * 注意：该函数消耗较大会创建新的物品实例，故只推荐在不同库存组件交换物品时使用。
	 *		对于相同库存组件下的物品交换，只需要更改物品实例内的信息即可。
	 * @param ItemDef 物品的定义
	 * @param InSlotTag 目标容器标签
	 * @param InStackCount 当前物品的堆叠数量 
	 * @param Args 可变参数列表，传入的参数会同一赋值给创建的物品实例
	 */
	template<class T = UInvSys_InventoryItemInstance, class... Arg>
	T* AddItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef,
		FGameplayTag InSlotTag,	int32 InStackCount, const Arg&... Args)
	{
		UInvSys_InventoryFragment_Container* ContainerFragment =
			FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(InSlotTag);
		if (ContainerFragment)
		{
			return ContainerFragment->AddItemDefinition<T>(ItemDef, InStackCount, Args...);
		}
		return nullptr;
	}
	template<class T, class... Arg>
	bool AddItemInstance(T* InItemInstance, FGameplayTag SlotTag, const Arg&... Args)
	{
		UInvSys_InventoryFragment_Container* ContainerFragment =
			FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(SlotTag);
		if (InItemInstance == nullptr || ContainerFragment == nullptr)
		{
			checkNoEntry()
			return false;
		}
		/**
		 * 判断物品实例之前所在的库存组件与当前组件是否一致
		 * 不一致时，说明物品实例的 Outer 不是当前组件的 Owner 所以需要重新复制一份并更新 Outer
		 */
		T* TargetItemInstance = InItemInstance;
		if (this != InItemInstance->GetInventoryComponent())
		{
			TargetItemInstance = DuplicateObject<T>(InItemInstance, this);
			InItemInstance->ConditionalBeginDestroy();//标记目标待删除
		}
		return ContainerFragment->AddItemInstance<T>(TargetItemInstance, Args...);
	}
	
	bool RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance);
	bool RestoreItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	// Begin Drag Drop ====================
	bool TryDragItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	void DropItemInstanceToWorld(UInvSys_InventoryItemInstance* InItemInstance);
	// End Drag Drop ====================

protected:
	/** 创建所有库存对象后被调用 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory Component")
	void OnConstructInventoryObjects();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory Component")
	void OnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance);
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory Component")
	void OnUnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance);
	
public:
	/**
	 * Getter Or Setter
	 **/
	template<class T>
	T* FindInventoryObjectFragment(FGameplayTag Tag) const
	{
		if (InventoryObjectMap.Contains(Tag))
		{
			return InventoryObjectMap[Tag]->FindInventoryFragment<T>();
		}
		return nullptr;
	}

	template<class T = UInvSys_BaseInventoryObject>
	T* FindInventoryObject(FGameplayTag Tag) const
	{
		if (InventoryObjectMap.Contains(Tag))
		{
			return InventoryObjectMap[Tag];
		}
		return nullptr;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (DeterminesOutputType = OutClass))
	UInvSys_BaseInventoryFragment* FindInventoryObjectFragment(FGameplayTag Tag, TSubclassOf<UInvSys_BaseInventoryFragment> OutClass) const;

	// UInvSys_EquipSlotWidget* GetInventorySlotWidget(FGameplayTag SlotTag);
	
	/** Returns true if the owner's role is ROLE_Authority */
	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE bool IsLocalController() const;

	FORCEINLINE APlayerController* GetPlayerController() const;

	virtual bool IsContainsInventoryItem(const FName ItemUniqueID);
	
	UInvSys_PreEditInventoryObject* GetPreEditInventoryObject(int32 Index) const;

protected:
	//~UObject interface
	virtual void ReadyForReplication() override;

	/**
	 * 复制到客户端的对象的顺序为
	 * 1、Inventory Fragments SubObjects
	 * 2、Inventory Fragments
	 * 3、Inventory Object
	 */
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface
	
protected:
	/** [Replication] 库存对象列表，由 InventoryContentMapping 管理，支持配置背包、装备槽或其他类型 */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryObjectList, BlueprintReadOnly, Category = "Inventory Component")
	TArray<UInvSys_BaseInventoryObject*> InventoryObjectList;
	UFUNCTION()
	void OnRep_InventoryObjectList();

	/** [Client & Server] 加速库存对象查询速度 */
	UPROPERTY()
	TMap<FGameplayTag, UInvSys_BaseInventoryObject*> InventoryObjectMap;
	
	/** [CDO]库存内容映射，使用数据资产方便管理 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TSoftClassPtr<class UInvSys_InventoryContentMapping> InventoryObjectContent;

	/** [Client] 拥有该组件的玩家控制器。 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Component")
	APlayerController* OwningPlayer;

	/** [Client] 库存对象是否已经更新？限制初始化函数只执行一次。 */
	bool bDisplayWidgetIsValid;

	/** 库存组件的展示菜单 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TSubclassOf<UInvSys_InventoryLayoutWidget> LayoutWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Component")
	TObjectPtr<UInvSys_InventoryLayoutWidget> LayoutWidget;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, Category = "Inventory Component")
	FOnDropItemInstanceToWorld OnDropItemInstanceToWorld;
};
