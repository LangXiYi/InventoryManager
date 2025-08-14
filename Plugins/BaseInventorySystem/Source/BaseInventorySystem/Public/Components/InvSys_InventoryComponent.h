// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "InventoryObject/InvSys_BaseInventoryObject.h"
#include "InventoryObject/Fragment/InvSys_InventoryFragment_Container.h"
#include "InventoryObject/Fragment/InvSys_InventoryFragment_Equipment.h"
#include "InvSys_InventoryComponent.generated.h"

class AInvSys_PickableItems;
class UInvSys_BaseInventoryObject;
class UInvSys_InventoryLayoutWidget;
class UInvSys_PreEditInventoryObject;
class UInvSys_InventoryContentMapping;
class UInvSys_InventoryItemDefinition;

UCLASS(Abstract, BlueprintType, meta=(BlueprintSpawnableComponent))
class BASEINVENTORYSYSTEM_API UInvSys_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UInvSys_InventoryControllerComponent;
	friend class AInvSys_PickableContainer;

public:
	UInvSys_InventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * 构建库存对象列表，推荐在 BeginPlay 阶段就调用该函数
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	virtual void ConstructInventoryObjects();

	/**
	 * 创建库存显示控件
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory Component")
	virtual UInvSys_InventoryLayoutWidget* CreateDisplayWidget(APlayerController* NewPlayerController);

	/**
	 * 根据物品定义信息装备物品
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	UInvSys_InventoryItemInstance* EquipItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef,
		FGameplayTag SlotTag, int32 StackCount = 1);

	/**
	 * 装备物品实例，当 PayloadItems 不为空且存在容器模块时，自动将 PayloadItems 的所有成员添加到该容器模块内。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	UInvSys_InventoryItemInstance* EquipItemInstance(UInvSys_InventoryItemInstance* ItemInstance, FGameplayTag SlotTag);

	/**
	 * 卸下装备模块的装备
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	bool UnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	/**
	 * 卸下对应标签的装备模块的装备
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	bool UnEquipItemInstanceByTag(const FGameplayTag& InventoryTag);

	/**
	 * 目标物品是否在组件内被装备？
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Component")
	bool IsEquippedItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	/**
	 * 对应标签的装备模块是否已经装备物品？
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Component")
	bool HasEquippedItemInstance(FGameplayTag InventoryTag);

	/**
	 * 根据物品定义创建物品并添加至当前容器内
	 * 注意：可变参数列表要求目标类型必须实现 InitItemInstanceProps 函数，且参数类型一致。
	 */
	template<class T, class... ArgList>
	T* AddItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef,
		FGameplayTag InventoryTag, int32 StackCount, const ArgList&... Args)
	{
		if (IsValidInventoryTag(InventoryTag) == false)
		{
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning,
				TEXT("%hs Falied, Is not valid tag %s."), __FUNCTION__, *InventoryTag.ToString())
			return nullptr;
		}
		auto ContainerFragment = FindInventoryModule<UInvSys_InventoryFragment_Container>(InventoryTag);
		if (ContainerFragment == nullptr)
		{
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning,
				TEXT("%hs Falied, ContainerFragment not find by %s."), __FUNCTION__, *InventoryTag.ToString())
			return nullptr;
		}
		return ContainerFragment->AddItemDefinition<T>(ItemDef, StackCount, Args...);
	}

	/**
	 * 添加物品至当前容器
	 * 注意：可变参数列表要求目标类型必须实现 InitItemInstanceProps 函数，且参数类型一致。
	 */
	template<class T, class... ArgList>
	T* AddItemInstance(UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag, const ArgList&... Args)
	{
		auto ContainerFragment = FindInventoryModule<UInvSys_InventoryFragment_Container>(SlotTag);
		if (ContainerFragment != nullptr)
		{
			return ContainerFragment->AddItemInstance<T>(InItemInstance, Args...);
		}
		return nullptr;
	}

	/** 移除指定物品 */
	bool RemoveItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	/** 移除指定标签下的物品实例 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	bool RemoveItemInstance(UInvSys_InventoryItemInstance* ItemInstance, FGameplayTag InventoryTag);

	/**
	 * 更新容器内物品的属性
	 * 注意：可变参数列表要求目标类型必须实现 InitItemInstanceProps 函数，且参数类型一致。
	 */
	template<class T, class... ArgList>
	void UpdateItemInstance(UInvSys_InventoryItemInstance* ItemInstance, FGameplayTag SlotTag, const ArgList&... Args)
	{
		auto ContainerFragment = FindInventoryModule<UInvSys_InventoryFragment_Container>(SlotTag);
		if (ContainerFragment != nullptr)
		{
			ContainerFragment->UpdateItemInstance<T>(ItemInstance, Args...);
		}
	}

	/** 修改物品堆叠数量 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	void UpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance, int32 NewStackCount);

	/** 修改物品拖拽状态 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	void UpdateItemDragState(UInvSys_InventoryItemInstance* ItemInstance, const FGameplayTag& InventoryTag, bool NewState);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	bool ContainsItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	/**
	 * 拖拽并在容器中删除该物品
	 * 注意：从容器内移除后，需要在其他位置手动同步该物品实例，否则属性修改不会同步值客户端。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	bool DragAndRemoveItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	/**
	 * 拖拽物品会将背包内的该物品锁住，必须在放下或取消拖拽后取消该锁定！！
	 * 可以通过 UInvSys_InventoryItemInstance::SetIsDraggingItem 修改锁定状态。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	bool DragItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	/** 取消拖拽物品 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	void CancelDragItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	/**
	 * 放下物品
	 * 存在两种不同执行流程：
	 * 第一种：修改对象属性值；
	 * 第二种：修改容器成员
	 *		A、将对象从原有的容器内取出后放置到新容器内。
	 *		B、加入新容器前需要判断物品所有者是否发生改变，以此决定是否进行深度拷贝并重设所有者。
	 */
	template<class T, class... Arg>
	bool DropItemInstance(UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag, const Arg&... Args);

	/**  丢弃物品到世界 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	AInvSys_PickableItems* DiscardItemInstance(UInvSys_InventoryItemInstance* InItemInstance, const FTransform& Transform);

protected:
	/** 创建所有库存对象后被调用 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory Component")
	void OnConstructInventoryObjects();
	
public:
	/**
	 * Getter Or Setter
	 **/
	template<class T>
	T* FindInventoryModule(FGameplayTag Tag) const
	{
		if (InventoryObjectMap.Contains(Tag))
		{
			return InventoryObjectMap[Tag]->FindInventoryFragment<T>();
		}
		UE_LOG(LogInventorySystem, Warning, TEXT("GameplayTag[%s] is not valid."), *Tag.ToString())
		return nullptr;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (DeterminesOutputType = OutClass))
	UInvSys_BaseInventoryFragment* FindInventoryFragment(FGameplayTag Tag, TSubclassOf<UInvSys_BaseInventoryFragment> OutClass) const;

	void RegisterInventoryComponent(
		const TSoftClassPtr<UInvSys_InventoryContentMapping>& InInventoryContent,
		const TSubclassOf<UInvSys_InventoryLayoutWidget>& InLayoutWidget);

	bool IsValidInventoryTag(const FGameplayTag& InventoryTag) const;

	/** Returns true if the owner's role is ROLE_Authority */
	bool HasAuthority() const;

	bool IsLocalController() const;

	APlayerController* GetPlayerController() const;

	UInvSys_InventoryObjectContent* GetInventoryObjectContent(int32 Index) const;

protected:
	/**
	 * 该顺序会影响 RepNotify 以及对象在客户端的生成顺序
	 * 1、Inventory Fragments SubObjects
	 * 2、Inventory Fragments
	 * 3、Inventory Object
	 */
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
};

template <class T, class ... Arg>
bool UInvSys_InventoryComponent::DropItemInstance(UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag,
	const Arg&... Args)
{
	check(InItemInstance)
	UInvSys_InventoryComponent* FromInvComp = InItemInstance->GetInventoryComponent();
	if (SlotTag == InItemInstance->GetInventoryObjectTag() && this == FromInvComp)
	{
		UpdateItemDragState(InItemInstance, SlotTag, false); 
		UpdateItemInstance<T>(InItemInstance, SlotTag, Args...);
		return true;
	}
	if (FromInvComp->RemoveItemInstance(InItemInstance))
	{
		AddItemInstance<T>(InItemInstance, SlotTag, Args...);
		return true;
	}
	return false;
}
