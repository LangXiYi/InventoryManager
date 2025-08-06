// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
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
	friend class AInvSys_PickableContainer;

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
	template<class T, class... Arg>
	T* AddItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef,
		FGameplayTag InSlotTag,	int32 InStackCount, const Arg&... Args)
	{
		auto ContainerFragment = FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(InSlotTag);
		if (ContainerFragment)
		{
			return ContainerFragment->AddItemDefinition<T>(ItemDef, InStackCount, Args...);
		}
		return nullptr;
	}

	template<class T, class... Arg>
	bool AddItemInstance(UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag, const Arg&... Args)
	{
		auto ContainerFragment = FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(SlotTag);
		if (InItemInstance != nullptr && ContainerFragment != nullptr)
		{
			return ContainerFragment->AddItemInstance<T>(InItemInstance, Args...);
		}
		return false;
	}

	bool RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance);
	bool RestoreItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	template<class T, class... Arg>
	bool UpdateItemInstance(UInvSys_InventoryItemInstance* ItemInstance, FGameplayTag SlotTag, const Arg&... Args)
	{
		auto ContainerFragment = FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(SlotTag);
		if (ItemInstance && ContainerFragment)
		{
			return ContainerFragment->UpdateItemInstance<T>(ItemInstance, Args...);
		}
		return false;
	}

	// Begin Drag Drop ====================
	bool DragAndRemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance);
	/**
	 * 拖拽物品会将背包内的该物品锁住，必须在放下或取消拖拽后取消该锁定！！
	 * 可以通过 UInvSys_InventoryItemInstance::SetIsDraggingItem 修改锁定状态。
	 */
	bool DragItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	/**
	 * 放下物品存在两种不同执行流程：
	 * 第一种：修改对象属性值；
	 * 第二种：修改容器成员
	 *		A、将对象从原有的容器内取出后放置到新容器内。
	 *		B、加入新容器前需要判断物品所有者是否发生改变，以此决定是否进行深度拷贝并重设所有者。
	 */
	template<class T, class... Arg>
	bool DropItemInstance(UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag, const Arg&... Args)
	{
		bool bIsSuccess = false;
		if (InItemInstance)
		{
			InItemInstance->SetIsDraggingItem(false); // 不能在 AddItemInstance 之后执行，避免复制之后修改旧对象的值！
			UInvSys_InventoryComponent* FromInvComp = InItemInstance->GetInventoryComponent();
			if (SlotTag == InItemInstance->GetSlotTag() && this == FromInvComp)
			{
				bIsSuccess = UpdateItemInstance<T>(InItemInstance, SlotTag, Args...);
			}
			else
			{
				FromInvComp->RemoveItemInstance(InItemInstance);
				bIsSuccess = AddItemInstance<T>(InItemInstance, SlotTag, Args...);
				check(bIsSuccess)
			}
		}
		return bIsSuccess;
	}

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
