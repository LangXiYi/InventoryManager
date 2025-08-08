// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InvSys_InventoryComponent.h"

#include "BaseInventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_DisplayWidget.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Equipment.h"
#include "Data/InvSys_InventoryContentMapping.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Data/InvSys_ItemFragment_EquipItem.h"
#include "Engine/ActorChannel.h"
#include "Engine/AssetManager.h"
#include "Items/InvSys_PickableItems.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/InvSys_InventoryLayoutWidget.h"
#include "Widgets/Components/InvSys_TagSlot.h"

UInvSys_InventoryComponent::UInvSys_InventoryComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	// 不开启该属性，开启该属性可能会出现子对象与FastArray的属性同步失序的问题。
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
	
	if (HasAuthority())
	{
		auto PostLoadInventoryObjectContent = [&]()
		{
			UInvSys_InventoryContentMapping* CDO_InventoryObjectContent =
				InventoryObjectContent->GetDefaultObject<UInvSys_InventoryContentMapping>();
			
			InventoryObjectList.Empty();
			InventoryObjectList.Reserve(CDO_InventoryObjectContent->InventoryContentList.Num());
			for (UInvSys_PreEditInventoryObject* PreEditInventoryObject : CDO_InventoryObjectContent->InventoryContentList)
			{
				if (PreEditInventoryObject == nullptr)
				{
					UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("库存对象内容映射 [InventoryContentMapping] 中存在空值."));
					continue;
				}
				UInvSys_BaseInventoryObject* InventoryObject = PreEditInventoryObject->NewInventoryObject(this);
				if (InventoryObject == nullptr)
				{
					UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("预设 [%s] 构建的库存对象为空."), *PreEditInventoryObject->GetName());
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
		// LayoutWidget->CollectAllTagSlots();
		// LayoutWidget->SetInventoryComponent(this);
		
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
			// 延迟下一帧执行刷新函数，确保控件创建流程执行完毕。
			// GetWorld()->GetTimerManager().SetTimerForNextTick([InvObj]()
			// {
			// 	// 创建控件完成后刷新一次显示效果
			// 	InvObj->RefreshInventoryFragment(UInvSys_InventoryFragment_DisplayWidget::StaticClass());
			// });
		}
	}
	return LayoutWidget;
}

bool UInvSys_InventoryComponent::RestoreItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance)
	{
		FGameplayTag EquipSlotTag = InItemInstance->GetSlotTag();
		UInvSys_InventoryFragment_Equipment* EquipmentFragment =
			FindInventoryObjectFragment<UInvSys_InventoryFragment_Equipment>(EquipSlotTag);

		UInvSys_InventoryFragment_Container* ContainerFragment =
			FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(EquipSlotTag);

		if (EquipmentFragment && EquipmentFragment->HasEquipmentItems() == false)
		{
			EquipmentFragment->EquipItemInstance(InItemInstance);
			// todo::如果ItemInstance类型不符合是否会装备失败？装备失败该如何处理?
		}
		else if (ContainerFragment) // todo:: Has enough space to save
		{
			// todo:: set Item instance position
			ContainerFragment->AddItemInstance<UInvSys_InventoryItemInstance>(InItemInstance);
		}
		else
		{
			//todo::Drop item to world;
		}
		return true;
	}
	return false;
}

bool UInvSys_InventoryComponent::UpdateItemInstanceDragState(UInvSys_InventoryItemInstance* ItemInstance,
	const FGameplayTag& InventoryTag, bool NewState)
{
	auto ContainerFragment = FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(InventoryTag);
	check(ContainerFragment)
	if (ContainerFragment)
	{
		return ContainerFragment->UpdateItemInstanceDragState(ItemInstance, NewState);
	}
	return false;
}

bool UInvSys_InventoryComponent::DragAndRemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance == nullptr) return false;

	auto DragDropFragment = InItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
	if (DragDropFragment == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("目标物品不支持拖拽功能"))
		return false;
	}

	FGameplayTag ItemTag = InItemInstance->GetSlotTag();
	//判断拖拽的物品是不是装备槽正在装备的物品
	auto EquipmentFragment = FindInventoryObjectFragment<UInvSys_InventoryFragment_Equipment>(ItemTag);
	if (EquipmentFragment && EquipmentFragment->GetEquipItemInstance() == InItemInstance)
	{
		return UnEquipItemInstance(InItemInstance);
	}

	//判断拖拽这个物品之前判断物品在它容器中是否存在
	UInvSys_InventoryFragment_Container* ContainerFragment =
		FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(ItemTag);
	if (ContainerFragment)
	{
		return RemoveItemInstance(InItemInstance);
	}
	return false;
}

bool UInvSys_InventoryComponent::DragItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	bool bIsSuccess = false;
	check(ItemInstance)
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
			bIsSuccess = UpdateItemInstanceDragState(ItemInstance, ItemInstance->GetSlotTag(), true);
		}
	}
#if WITH_EDITOR
	UE_CLOG(bIsSuccess == false, LogInventorySystem, Warning, TEXT("[%s]拖拽物品失败:物品实例已经被其他玩家拖拽"),
		*GPlayInEditorContextString);
#endif
	return bIsSuccess;
}

void UInvSys_InventoryComponent::CancelDragItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	check(ItemInstance)
	if (ItemInstance != nullptr && IsValid(ItemInstance))
	{
		if (ItemInstance->IsDraggingItemInstance() == false)
		{
			UpdateItemInstanceDragState(ItemInstance, ItemInstance->GetSlotTag(), false);
		}
	}
}

void UInvSys_InventoryComponent::DropItemInstanceToWorld(UInvSys_InventoryItemInstance* InItemInstance)
{
	check(InItemInstance)
	if (InItemInstance)
	{
		// RemoveItemInstance(InItemInstance);
		auto DropItemFragment = InItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
		if (DropItemFragment && DropItemFragment->DropItemClass)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();

			AInvSys_PickableItems* PickableItems = GetWorld()->SpawnActor<AInvSys_PickableItems>(DropItemFragment->DropItemClass, SpawnParameters);
			PickableItems->InitItemInstance(InItemInstance);
			
			// bug::在拖拽宝箱内的物品时，无法正确获取到玩家角色！！！所以丢弃的位置会出现错误。
			OnDropItemInstanceToWorld.Broadcast(PickableItems);
		}
		else
		{
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("丢弃的物品可能不存在 DragDrop 片段！需要在物品定义中添加并定义属性。"))
		}
	}
}

void UInvSys_InventoryComponent::EquipItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, FGameplayTag SlotTag)
{
	auto EquipmentFragment = FindInventoryObjectFragment<UInvSys_InventoryFragment_Equipment>(SlotTag);
	if (EquipmentFragment != nullptr)
	{
		UInvSys_InventoryItemInstance* TargetItemInstance = EquipmentFragment->EquipItemDefinition(ItemDef);
		OnEquipItemInstance(TargetItemInstance);
	}
	else
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("对应标记 %s 的库存对象没有设置装备片段，故执行装备物品失败。"), *SlotTag.ToString());
	}
}

bool UInvSys_InventoryComponent::EquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag)
{
	if (InItemInstance == nullptr)
	{
		return false;
	}
	// 如果装备的物品如果是来自其他对象或组件，则需要将物品及其内部所有对象拷贝并重设为当前的库存组件
	UInvSys_InventoryItemInstance* TargetItemInstance = InItemInstance;
	if (this != TargetItemInstance->GetInventoryComponent())
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品是从其他库存组件中转移至当前组件中的，现在正在进行所有权的转移。"))
		// 复制新的目标对象，并重设他的 Outer 为当前组件的所有者
		TargetItemInstance = DuplicateObject(InItemInstance, this);
		if (TargetItemInstance->MyInstances.Num() > 0)
		{
			// 内部包含了其他物品，这些物品的 Outer 同样不是该组件的 Owner 所以也需要重设它们的 Outer
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("\t物品内部还包含了其他物品，正在一起转移。"))
			TargetItemInstance->MyInstances.Empty();
			TargetItemInstance->MyInstances.Reserve(InItemInstance->MyInstances.Num());
			for (UInvSys_InventoryItemInstance* ContainerEntry : InItemInstance->MyInstances)
			{
				UInvSys_InventoryItemInstance* TempEntry = DuplicateObject(ContainerEntry, this);
				TargetItemInstance->MyInstances.Add(TempEntry);
				ContainerEntry->ConditionalBeginDestroy(); // 通知 GC 清理对象
			}
		}
		InItemInstance->MyInstances.Empty();
		InItemInstance->ConditionalBeginDestroy(); // 通知 GC 清理对象
	}

	UInvSys_InventoryFragment_Equipment* EquipmentFragment =
		FindInventoryObjectFragment<UInvSys_InventoryFragment_Equipment>(SlotTag);
		
	if (EquipmentFragment != nullptr && EquipmentFragment->HasEquipmentItems() == false)
	{
		EquipmentFragment->EquipItemInstance(TargetItemInstance);
		if (TargetItemInstance->MyInstances.Num() > 0)
		{
			UInvSys_InventoryFragment_Container* ContainerFragment =
				FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(SlotTag);
			if (ContainerFragment) // 转移装备内部包含的所有物品
			{
				UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("物品内部包含了[%d]个其他物品，现在正在对其内部物品进行转移。"),
					TargetItemInstance->MyInstances.Num())

				for (UInvSys_InventoryItemInstance* TempItem : TargetItemInstance->MyInstances)
				{
					ContainerFragment->AddItemInstance<UInvSys_InventoryItemInstance>(TempItem);
				}
				TargetItemInstance->MyInstances.Empty();
			}
		}
		OnEquipItemInstance(TargetItemInstance);
		return true;
	}
	return false;
}

bool UInvSys_InventoryComponent::UnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance)
	{
		UInvSys_InventoryFragment_Equipment* EquipmentFragment =
			FindInventoryObjectFragment<UInvSys_InventoryFragment_Equipment>(InItemInstance->GetSlotTag());
		
		if (EquipmentFragment)
		{
			EquipmentFragment->UnEquipItemInstance();
		}

		UInvSys_InventoryFragment_Container* ContainerFragment =
			FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(InItemInstance->GetSlotTag());
		if (ContainerFragment)
		{
			TArray<UInvSys_InventoryItemInstance*> AllItemInstance = ContainerFragment->GetAllItemInstance();
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("卸下的装备是一个容器，正在保存其内部物品，数量 = %d"), AllItemInstance.Num())
			// 将所有物品转移至该物品实例的内部
			InItemInstance->MyInstances.Empty();
			InItemInstance->MyInstances.Append(AllItemInstance);
			ContainerFragment->RemoveAllItemInstance();
		}

		OnUnEquipItemInstance(InItemInstance);
		return true;
	}
	return false;
}

bool UInvSys_InventoryComponent::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	check(InItemInstance)
	if (InItemInstance)
	{
		UInvSys_InventoryFragment_Container* ContainerFragment =
			FindInventoryObjectFragment<UInvSys_InventoryFragment_Container>(InItemInstance->GetSlotTag());
		check(ContainerFragment)
		if (ContainerFragment)
		{
			return ContainerFragment->RemoveItemInstance(InItemInstance);
		}
	}
	return false;
}

UInvSys_BaseInventoryFragment* UInvSys_InventoryComponent::FindInventoryObjectFragment(FGameplayTag Tag,
	TSubclassOf<UInvSys_BaseInventoryFragment> OutClass) const
{
	return FindInventoryObjectFragment<UInvSys_BaseInventoryFragment>(Tag);
}

/*UInvSys_EquipSlotWidget* UInvSys_InventoryComponent::GetInventorySlotWidget(FGameplayTag SlotTag)
{
	UInvSys_TagSlot* Slot = LayoutWidget->FindTagSlot(SlotTag);
	check(Slot)
	return Slot ? Cast<UInvSys_EquipSlotWidget>(Slot->GetChildAt(0)) : nullptr;
}*/

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

bool UInvSys_InventoryComponent::IsContainsInventoryItem(const FName ItemUniqueID)
{
	for (UInvSys_BaseInventoryObject* InventoryObject : InventoryObjectList)
	{
		/*if (InventoryObject->ContainsItem(ItemUniqueID))
		{
			return true;
		}*/
	}
	return false;
}

bool UInvSys_InventoryComponent::IsLocalController() const
{
	ensure(OwningPlayer);
	return OwningPlayer ? OwningPlayer->IsLocalController() : false;
}

UInvSys_PreEditInventoryObject* UInvSys_InventoryComponent::GetPreEditInventoryObject(int32 Index) const
{
	if (InventoryObjectContent.IsValid())
	{
		UInvSys_InventoryContentMapping* CDO_InventoryObjectContent =
			InventoryObjectContent->GetDefaultObject<UInvSys_InventoryContentMapping>();
		if (CDO_InventoryObjectContent->InventoryContentList.IsValidIndex(Index))
		{
			return CDO_InventoryObjectContent->InventoryContentList[Index];
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

void UInvSys_InventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing ULyraInventoryItemInstance
	/*if (IsUsingRegisteredSubObjectList() == false)
	{
		return;
	}
	
	for (UInvSys_BaseInventoryObject* InventoryObject : InventoryObjectList)
	{
		if (InventoryObject == nullptr || IsValid(InventoryObject) == false)
		{
			continue;
		}
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("初始化复制"))
		AddReplicatedSubObject(InventoryObject);
		if (InventoryObject->IsA(UInvSys_BaseEquipContainerObject::StaticClass()))
		{
			UInvSys_BaseEquipContainerObject* ContainerObject = Cast<UInvSys_BaseEquipContainerObject>(InventoryObject);
			for (FInvSys_ContainerEntry& Entry : ContainerObject->ContainerList.Entries)
			{
				if (Entry.Instance == nullptr || IsValid(Entry.Instance) == false)
				{
					continue;
				}
				UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("初始化复制 ===》 容器项"))
				AddReplicatedSubObject(Entry.Instance);
			}
		}
	}*/
}

void UInvSys_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_InventoryComponent, InventoryObjectList);
}

void UInvSys_InventoryComponent::OnRep_InventoryObjectList()
{
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("[%s:%s] 正在初始化库存对象列表[%d] "),
		HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwner()->GetName(), InventoryObjectList.Num())
	for (int i = 0; i < InventoryObjectList.Num(); ++i)
	{
		check(InventoryObjectList[i]);
		UInvSys_PreEditInventoryObject* PreEditInventoryObject = GetPreEditInventoryObject(i);
		if (InventoryObjectList[i] && PreEditInventoryObject)
		{
			InventoryObjectList[i]->InitInventoryObject(GetPreEditInventoryObject(i));
			InventoryObjectMap.Add(InventoryObjectList[i]->GetInventoryObjectTag(), InventoryObjectList[i]);
		}
	}
	OnConstructInventoryObjects();
	// 卸载库存数据的资源
	if (InventoryObjectContent.IsValid())
	{
		UAssetManager::GetStreamableManager().Unload(InventoryObjectContent.ToSoftObjectPath());
	}
}