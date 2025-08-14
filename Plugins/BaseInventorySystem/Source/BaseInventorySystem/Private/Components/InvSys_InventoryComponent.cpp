// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InvSys_InventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Engine/AssetManager.h"
#include "BaseInventorySystem.h"
#include "Items/InvSys_PickableItems.h"
#include "Data/InvSys_InventoryContentMapping.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_BaseItem.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Widgets/InvSys_InventoryLayoutWidget.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_DisplayWidget.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Equipment.h"

UInvSys_InventoryComponent::UInvSys_InventoryComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	bReplicateUsingRegisteredSubObjectList = false;
	//bReplicateUsingRegisteredSubObjectList = true; // 不推荐使用，否则可能会出现子对象与FastArray的属性同步失序的问题。
}

void UInvSys_InventoryComponent::ConstructInventoryObjects()
{
	// 权威服务器更新库存对象列表与映射关系
	if (InventoryObjectContent.IsNull() == true)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error, TEXT("The InventoryObjectsContent is nullptr."));
		return;
	}
	check(HasAuthority())
	auto PostLoadInventoryObjectContent = [this]()
	{
		UInvSys_InventoryContentMapping* CDO_InventoryObjectContent =
			InventoryObjectContent->GetDefaultObject<UInvSys_InventoryContentMapping>();

		InventoryObjectList.Empty();
		InventoryObjectList.Reserve(CDO_InventoryObjectContent->InventoryContentMapping.Num());
		for (const FInvSys_InventoryObjectHelper& InventoryObjectHelper : CDO_InventoryObjectContent->InventoryContentMapping)
		{
			UInvSys_BaseInventoryObject* InventoryObject = InventoryObjectHelper.ConstructInventoryObject(this);
			if (InventoryObject == nullptr)
			{
				UE_LOG(LogInventorySystem, Error, TEXT("预设构建的库存对象为空 -- %s."), *InventoryObjectHelper.InventoryObjectTag.ToString());
				continue;
			}
			InventoryObjectList.Emplace(InventoryObject);
		}
		if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
		{
			OnRep_InventoryObjectList();
		}
		// todo::读取数据库资源初始化库存数据？
		OnConstructInventoryObjects();
	};

	// 异步加载资产
	if (InventoryObjectContent.IsValid() == false)
	{
		FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
		StreamableManager.RequestSyncLoad(InventoryObjectContent.ToSoftObjectPath()); // 可根据需要决定是否使用异步节点
		// StreamableManager.RequestSyncLoad(InventoryObjectContent.ToSoftObjectPath(), FStreamableDelegate::CreateLambda(PostLoadInventoryObjectContent));
		PostLoadInventoryObjectContent();
	}
	else
	{
		PostLoadInventoryObjectContent();
	}
}

void UInvSys_InventoryComponent::RegisterInventoryComponent(
	const TSoftClassPtr<UInvSys_InventoryContentMapping>& InInventoryContent,
	const TSubclassOf<UInvSys_InventoryLayoutWidget>& InLayoutWidgetClass)
{
	InventoryObjectContent = InInventoryContent;
	LayoutWidgetClass = InLayoutWidgetClass;
	RegisterComponent();
}

bool UInvSys_InventoryComponent::IsValidInventoryTag(const FGameplayTag& InventoryTag) const
{
	if (InventoryTag.IsValid())
	{
		return InventoryObjectMap.Contains(InventoryTag);
	}
	return false;
}

UInvSys_InventoryLayoutWidget* UInvSys_InventoryComponent::CreateDisplayWidget(APlayerController* NewPlayerController)
{
	if (bDisplayWidgetIsValid)
	{
		return LayoutWidget;
	}
	if (NewPlayerController && NewPlayerController->IsLocalController() && LayoutWidgetClass)
	{
		bDisplayWidgetIsValid = true;
		OwningPlayer = NewPlayerController;
		
		// 创建布局控件后，收集所有的 TagSlot 供后续控件插入正确位置。
		LayoutWidget = CreateWidget<UInvSys_InventoryLayoutWidget>(NewPlayerController, LayoutWidgetClass);
		check(LayoutWidget)
		
		for (int Index = 0; Index < InventoryObjectList.Num(); ++Index)
		{
			UInvSys_BaseInventoryObject* InvObj = InventoryObjectList[Index];
			if (InvObj == nullptr)
			{
				checkNoEntry();
				continue;
			}
			auto DisplayWidgetFragment = InvObj->FindInventoryFragment<UInvSys_InventoryFragment_DisplayWidget>();
			if (DisplayWidgetFragment)
			{
				// 将库存对象的控件插入对应位置的插槽。
				UUserWidget* DisplayWidget = DisplayWidgetFragment->CreateDisplayWidget(NewPlayerController);
				LayoutWidget->AddWidget(DisplayWidget, InvObj->GetInventoryObjectTag());
			}
		}
	}
	return LayoutWidget;
}

void UInvSys_InventoryComponent::UpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance, int32 NewStackCount)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}
	auto ContainerFragment = FindInventoryModule<UInvSys_InventoryFragment_Container>(ItemInstance->GetInventoryObjectTag());
	if (ContainerFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ContainerFragment is nullptr."), __FUNCTION__)
		return;
	}
	ContainerFragment->UpdateItemStackCount(ItemInstance, NewStackCount);
}

void UInvSys_InventoryComponent::UpdateItemDragState(UInvSys_InventoryItemInstance* ItemInstance,
                                                     const FGameplayTag& InventoryTag, bool NewState)
{
	auto ContainerFragment = FindInventoryModule<UInvSys_InventoryFragment_Container>(InventoryTag);
	check(ContainerFragment)
	if (ContainerFragment)
	{
		ContainerFragment->UpdateItemInstanceDragState(ItemInstance, NewState);
	}
}

bool UInvSys_InventoryComponent::ContainsItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr)
	{
		return false;
	}
	auto ContainerFragment = FindInventoryModule<UInvSys_InventoryFragment_Container>(ItemInstance->GetInventoryObjectTag());
	if (ContainerFragment == nullptr)
	{
		return false;
	}
	return ContainerFragment->ContainsItem(ItemInstance);
}

bool UInvSys_InventoryComponent::DragAndRemoveItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, ItemInstance is nullptr.."), __FUNCTION__);
		return false;
	}

	if (ItemInstance->IsDraggingItemInstance() == true)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 物品实例已经被其他玩家拖拽."), __FUNCTION__);
		return false;
	}

	auto DragDropFragment = ItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
	if (DragDropFragment == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("目标物品不支持拖拽功能"))
		return false;
	}

	FGameplayTag InventoryTag = ItemInstance->GetInventoryObjectTag();
	//判断拖拽的物品是不是装备槽正在装备的物品
	auto EquipmentFragment = FindInventoryModule<UInvSys_InventoryFragment_Equipment>(InventoryTag);
	if (EquipmentFragment && EquipmentFragment->GetEquipItemInstance() == ItemInstance)
	{
		UpdateItemDragState(ItemInstance, InventoryTag, true);
		return UnEquipItemInstance(ItemInstance);
	}

	//判断拖拽这个物品之前判断物品在它容器中是否存在
	UInvSys_InventoryFragment_Container* ContainerFragment =
		FindInventoryModule<UInvSys_InventoryFragment_Container>(InventoryTag);
	if (ContainerFragment)
	{
		UpdateItemDragState(ItemInstance, InventoryTag, true);
		return RemoveItemInstance(ItemInstance);
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, %s is not valid."), __FUNCTION__, *InventoryTag.ToString());
	return false;
}

bool UInvSys_InventoryComponent::DragItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	check(ItemInstance);
	if (ItemInstance != nullptr && IsValid(ItemInstance))
	{
		/**
		 * 在网络环境较差的情况下，玩家A短时间内对物品进行连续操作，
		 * A:如果操作目的是将物品转移至其他容器，
		 *		由于物品实例会重新拷贝，所以玩家A仅第一次操作有效，后续步骤都是在对旧对象进行操作
		 *		Drop函数只会对拷贝后的新对象解锁，所以其他拖拽操作都会被阻挡
		 * B:如果操作目的是将物品的位置发生改变，并未改变物品的库存组件
		 *		不会发生拷贝，但在第一次操作改变位置后，后续的操作同样会被服务器执行，将最终结果发送给客户端
		 */
		if (ItemInstance->IsDraggingItemInstance() == false)
		{
			UpdateItemDragState(ItemInstance, ItemInstance->GetInventoryObjectTag(), true);
			return true;
		}
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 物品实例已经被其他玩家拖拽."), __FUNCTION__);
	}
	return false;
}

void UInvSys_InventoryComponent::CancelDragItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	check(ItemInstance)
	if (ItemInstance != nullptr && IsValid(ItemInstance))
	{
		UpdateItemDragState(ItemInstance, ItemInstance->GetInventoryObjectTag(), false);
	}
}

AInvSys_PickableItems* UInvSys_InventoryComponent::DiscardItemInstance(
	UInvSys_InventoryItemInstance* InItemInstance, const FTransform& Transform)
{
	check(InItemInstance)
	if (InItemInstance)
	{
		auto DropItemFragment = InItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
		if (DropItemFragment == nullptr)
		{
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品不存在 DragDrop 片段！需要在物品定义中添加并定义属性。"))
			return nullptr;
		}
		// checkf(DropItemFragment, TEXT("DropItemFragment is nullptr."));
		checkf(DropItemFragment->DropItemClass, TEXT("DropItemFragment->DropItemClass is nullptr"));
		if (IsEquippedItemInstance(InItemInstance))
		{
			UnEquipItemInstance(InItemInstance);
		}
		else
		{
			RemoveItemInstance(InItemInstance);
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();

		AInvSys_PickableItems* PickableItems = GetWorld()->SpawnActor<AInvSys_PickableItems>(
			DropItemFragment->DropItemClass, Transform, SpawnParameters);
		check(PickableItems)
		PickableItems->InitPickableItemInstance(InItemInstance);
		return PickableItems;
	}
	return nullptr;
}

UInvSys_InventoryItemInstance* UInvSys_InventoryComponent::EquipItemDefinition(
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, FGameplayTag SlotTag, int32 StackCount)
{
	auto EquipmentFragment = FindInventoryModule<UInvSys_InventoryFragment_Equipment>(SlotTag);
	if (EquipmentFragment == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("对应标记 %s 的库存对象没有设置装备片段，故执行装备物品失败。"), *SlotTag.ToString());
		return nullptr;
	}

	UInvSys_InventoryItemInstance* TargetItemInstance = EquipmentFragment->EquipItemDefinition(ItemDef);
	if (TargetItemInstance == nullptr)
	{
		return nullptr;
	}
	return TargetItemInstance;
}

UInvSys_InventoryItemInstance* UInvSys_InventoryComponent::EquipItemInstance(
	UInvSys_InventoryItemInstance* ItemInstance, FGameplayTag SlotTag)
{
	auto EquipmentFragment = FindInventoryModule<UInvSys_InventoryFragment_Equipment>(SlotTag);
	if (EquipmentFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, EquipmentFragment is nullptr."), __FUNCTION__)
		return nullptr;
	}
	UInvSys_InventoryItemInstance* EquipmentInstance = EquipmentFragment->EquipItemInstance(ItemInstance);
	if (EquipmentInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, EquipmentInstance is nullptr."), __FUNCTION__)
		return nullptr;
	}

	if (EquipmentInstance->PayloadItems.IsEmpty() == false)
	{
		/**
		 * 将 PayloadItems 的所有成员添加到该容器模块内。
		 */
		auto ContainerFragment = FindInventoryModule<UInvSys_InventoryFragment_Container>(SlotTag);
		checkf(ContainerFragment, TEXT("ContainerFragment 不应该为空。当 PayloadItems 不为空时，则说明该物品应该是一个带有容器的物品."))
		ContainerFragment->RemoveAllItemInstance();

		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
			TEXT("物品内部包含了[%d]个其他物品，现在正在对其内部物品进行转移。"), EquipmentInstance->PayloadItems.Num())
		for (UInvSys_InventoryItemInstance* TempItem : EquipmentInstance->PayloadItems)
		{
			// todo::这里需要用户自定义逻辑处理这部分转移的物品实例
			// TempItem->ItemPosition.SlotTag = SlotTag;
			TempItem->OnTransferItems(ContainerFragment);
			ContainerFragment->AddItemInstance<UInvSys_InventoryItemInstance>(TempItem);
		}
		EquipmentInstance->PayloadItems.Empty();
	}
	return EquipmentInstance;
}

bool UInvSys_InventoryComponent::UnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, InItemInstance is nullptr."), __FUNCTION__)
		return false;
	}
	return UnEquipItemInstanceByTag(InItemInstance->GetInventoryObjectTag());
}

bool UInvSys_InventoryComponent::UnEquipItemInstanceByTag(const FGameplayTag& InventoryTag)
{
	auto EquipmentFragment = FindInventoryModule<UInvSys_InventoryFragment_Equipment>(InventoryTag);
	if (EquipmentFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, EquipmentFragment is nullptr, GameplayTag = %s"), __FUNCTION__, *InventoryTag.ToString())
		return false;
	}
	if (EquipmentFragment->HasEquipmentItems() == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, HasEquipmentItems is false, GameplayTag = %s"), __FUNCTION__, *InventoryTag.ToString())
		return false;
	}

	// 获取容器模块在暂存所有物品后清空容器
	UInvSys_InventoryItemInstance* ItemInstance = EquipmentFragment->GetEquipItemInstance();
	auto ContainerFragment = FindInventoryModule<UInvSys_InventoryFragment_Container>(InventoryTag);
	if (ContainerFragment != nullptr)
	{
		ContainerFragment->GetAllItemInstance(ItemInstance->PayloadItems);
		ContainerFragment->RemoveAllItemInstance();
	}

	return EquipmentFragment->UnEquipItemInstance();
}

bool UInvSys_InventoryComponent::IsEquippedItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return false;
	}
	auto EquipmentFragment = FindInventoryModule<UInvSys_InventoryFragment_Equipment>(ItemInstance->GetInventoryObjectTag());
	if (EquipmentFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, EquipmentFragment is nullptr."), __FUNCTION__)
		return false;
	}

	return EquipmentFragment->GetEquipItemInstance() == ItemInstance;
}

bool UInvSys_InventoryComponent::HasEquippedItemInstance(FGameplayTag InventoryTag)
{
	if (IsValidInventoryTag(InventoryTag) == false)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, InventoryTag is not valid."), __FUNCTION__)
		return false;
	}
	auto EquipmentFragment = FindInventoryModule<UInvSys_InventoryFragment_Equipment>(InventoryTag);
	if (EquipmentFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, EquipmentFragment is nullptr."), __FUNCTION__)
		return false;
	}
	return EquipmentFragment->HasEquipmentItems();
}

bool UInvSys_InventoryComponent::RemoveItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return false;
	}
	return RemoveItemInstance(ItemInstance, ItemInstance->GetInventoryObjectTag());
}

bool UInvSys_InventoryComponent::RemoveItemInstance(UInvSys_InventoryItemInstance* ItemInstance,
	FGameplayTag InventoryTag)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return false;
	}
	auto ContainerFragment = FindInventoryModule<UInvSys_InventoryFragment_Container>(InventoryTag);
	if (ContainerFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, 在 %s 的库存组件中不存在片段 %s."),
			__FUNCTION__, *GetOwner()->GetName(), *InventoryTag.ToString())
		return false;
	}

	return ContainerFragment->RemoveItemInstance(ItemInstance);
}

UInvSys_BaseInventoryFragment* UInvSys_InventoryComponent::FindInventoryFragment(FGameplayTag Tag,
                                                                                       TSubclassOf<UInvSys_BaseInventoryFragment> OutClass) const
{
	if (IsValidInventoryTag(Tag))
	{
		return InventoryObjectMap[Tag]->FindInventoryFragment(OutClass);
	}
	return nullptr;
}

bool UInvSys_InventoryComponent::HasAuthority() const
{
	ensure(GetOwner());
	return GetOwner() ? GetOwner()->HasAuthority() : false;
}

APlayerController* UInvSys_InventoryComponent::GetPlayerController() const
{
	ensure(OwningPlayer);
	return OwningPlayer;
}

bool UInvSys_InventoryComponent::IsLocalController() const
{
	ensure(OwningPlayer);
	return OwningPlayer ? OwningPlayer->IsLocalController() : false;
}

UInvSys_InventoryObjectContent* UInvSys_InventoryComponent::GetInventoryObjectContent(int32 Index) const
{
	if (InventoryObjectContent.IsValid())
	{
		UInvSys_InventoryContentMapping* CDO_InventoryObjectContent =
			InventoryObjectContent->GetDefaultObject<UInvSys_InventoryContentMapping>();
		if (CDO_InventoryObjectContent->InventoryContentMapping.IsValidIndex(Index))
		{
			return CDO_InventoryObjectContent->InventoryContentMapping[Index].InventoryObjectContent;
		}
		checkf(false, TEXT("传入的 Index 无效"));
	}
	return nullptr;
}

bool UInvSys_InventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                     FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UInvSys_BaseInventoryObject* InventoryObject : InventoryObjectList)
	{
		if (InventoryObject == nullptr || IsValid(InventoryObject) == false)
		{
			continue;
		}
		WroteSomething |= InventoryObject->ReplicateSubobjects(Channel, Bunch, RepFlags);
		WroteSomething |= Channel->ReplicateSubobject(InventoryObject, *Bunch, *RepFlags);
	}
	return WroteSomething;
}

void UInvSys_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_InventoryComponent, InventoryObjectList);
}

void UInvSys_InventoryComponent::OnRep_InventoryObjectList()
{
	if (InventoryObjectContent.IsValid() == false)
	{
		FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
		StreamableManager.RequestSyncLoad(InventoryObjectContent.ToSoftObjectPath()); // 可根据需要决定是否使用异步节点
	}
	for (int i = 0; i < InventoryObjectList.Num(); ++i)
	{
		check(InventoryObjectList[i]);
		InventoryObjectList[i]->InitInventoryObject(this, i);
		InventoryObjectMap.Add(InventoryObjectList[i]->GetInventoryObjectTag(), InventoryObjectList[i]);
	}
	OnConstructInventoryObjects();
	// 卸载库存数据的资源
	if (InventoryObjectContent.IsValid())
	{
		UAssetManager::GetStreamableManager().Unload(InventoryObjectContent.ToSoftObjectPath());
	}
}