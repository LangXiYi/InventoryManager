// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InvSys_InventoryComponent.h"

#include "BaseInventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
#include "Data/InvSys_InventoryContentMapping.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_DragDrop.h"
#include "Data/InvSys_ItemFragment_PickUpItem.h"
#include "Engine/ActorChannel.h"
#include "Engine/AssetManager.h"
#include "Interface/InvSys_DraggingItemInterface.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/InvSys_EquipSlotWidget.h"
#include "Widgets/InvSys_InventoryLayoutWidget.h"
#include "Widgets/Components/InvSys_TagSlot.h"

#if WITH_EDITOR
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#endif

UInvSys_InventoryComponent::UInvSys_InventoryComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	// 不开启该属性，开启该属性可能会出现子对象与FastArray的属性同步失序的问题。
	bReplicateUsingRegisteredSubObjectList = false;
	//bReplicateUsingRegisteredSubObjectList = true; // 不推荐使用，否则可能会出现子对象与FastArray的属性同步失序的问题。
}

void UInvSys_InventoryComponent::RestoreItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	// 还原拖拽的物品信息
	// DraggingItemInstance = nullptr;
	
	check(InItemInstance)
	check(InItemInstance->GetInventoryComponent() == this)
	if (InItemInstance && InItemInstance->GetInventoryComponent() == this)
	{
		FGameplayTag EquipSlotTag = InItemInstance->GetSlotTag();
		UInvSys_BaseInventoryObject* InventoryObject = GetInventoryObject(EquipSlotTag);
		if (InventoryObject)
		{
			bool bIsSuccess = InventoryObject->RestoreItemInstance(InItemInstance);
			check(bIsSuccess);
		}
	}
}

bool UInvSys_InventoryComponent::TryDragItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance == nullptr) return false; // 仅在未拖拽其它物品时可以拖拽。

	auto DragDropFragment = InItemInstance->FindFragmentByClass<UInvSys_ItemFragment_DragDrop>();
	if (DragDropFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("目标物品不支持拖拽功能"))
		return false;
	}
	
	UInvSys_InventoryComponent* LOCAL_InvComp = InItemInstance->GetInventoryComponent<UInvSys_InventoryComponent>();
	if (LOCAL_InvComp == nullptr) return false;

	//将物品从它所在的容器中移除
	LOCAL_InvComp->RemoveItemInstance(InItemInstance);
	/*if (LOCAL_InvComp != this) //不同库存组件的物品拖拽会生成一个新的物品给客户端，然后销毁传入的物品实例
	{
		UInvSys_InventoryItemInstance* LOCAL_ItemInstance = InItemInstance; // 暂存目标指针
		InItemInstance = DuplicateObject(InItemInstance, GetOwner()); // 复制一个新的物品实例，将其Outer指定为当前组件的Owner
		LOCAL_ItemInstance->ConditionalBeginDestroy(); //标记目标物品待销毁
	}*/
	//DraggingItemInstance = InItemInstance; //通过RepNotify通知客户端
	/*if (GetNetMode() != NM_DedicatedServer)
	{
		OnRep_DraggingItemInstance();
	}*/
	return true;
}

void UInvSys_InventoryComponent::CancelDragItemInstance()
{
	/*if (DraggingItemInstance)
	{
		DraggingItemInstance = nullptr;
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnRep_DraggingItemInstance();
		}
	}*/
}

void UInvSys_InventoryComponent::NativeOnDiscardItemInstance(UInvSys_InventoryItemInstance* InDraggingItemInstance)
{
	UE_LOG(LogInventorySystem, Error, TEXT("正在尝试丢弃该物品"))
	auto PickUpItemFragment = InDraggingItemInstance->FindFragmentByClass<UInvSys_ItemFragment_PickUpItem>();
	if (PickUpItemFragment)
	{
		//todo::创建可拾取的物品
		// 广播创建可拾取物品
	}
}

void UInvSys_InventoryComponent::NativeOnDraggingItemInstance(UInvSys_InventoryItemInstance* InDraggingItemInstance)
{
	if (InDraggingItemInstance && DraggingWidget)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("修正客户端的拖拽的目标物品"))
		IInvSys_DraggingItemInterface::Execute_UpdateItemInstance(DraggingWidget, InDraggingItemInstance);
	}
}

void UInvSys_InventoryComponent::NativeOnCancelDragItemInstance()
{
	if (DraggingWidget)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("客户端取消拖拽了"))
		IInvSys_DraggingItemInterface::Execute_OnCancelDragItemInstance(DraggingWidget);
	}
}

void UInvSys_InventoryComponent::EquipItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, FGameplayTag SlotTag)
{
	UInvSys_BaseEquipmentObject* EquipObj = GetInventoryObject<UInvSys_BaseEquipmentObject>(SlotTag);
	if (EquipObj != nullptr)
	{
		UInvSys_InventoryItemInstance* LOCAL_ItemInstance = EquipObj->EquipItemDefinition(ItemDef);
		if (LOCAL_ItemInstance)
		{
			OnEquipItemInstance(LOCAL_ItemInstance);
		}
	}
}

void UInvSys_InventoryComponent::EquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag)
{
	UInvSys_InventoryItemInstance* TargetItemInstance = InItemInstance;
	//装备的物品如果是来自其他库存组件
	if (TargetItemInstance == nullptr)
	{
		return;
	}

	if (this != TargetItemInstance->GetInventoryComponent())
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("物品是从其他库存组件中转移至当前组件中的，现在正在进行所有权的转移。"))
		// 复制新的目标对象，并重设他的 Outer 为当前组件的所有者
		TargetItemInstance = DuplicateObject(InItemInstance, GetOwner());
		if (TargetItemInstance->MyInstances.Num() > 0)
		{
			UE_LOG(LogInventorySystem, Warning, TEXT("\t物品内部还包含了其他物品，正在一起转移。"))
			TargetItemInstance->MyInstances.Empty();
			TargetItemInstance->MyInstances.Reserve(InItemInstance->MyInstances.Num());
			for (UInvSys_InventoryItemInstance* ContainerEntry : InItemInstance->MyInstances)
			{
				UInvSys_InventoryItemInstance* TempEntry = DuplicateObject(ContainerEntry, GetOwner());
				TargetItemInstance->MyInstances.Add(TempEntry);
				ContainerEntry->ConditionalBeginDestroy(); // 通知 GC 清理对象
			}
		}
		InItemInstance->MyInstances.Empty();
		InItemInstance->ConditionalBeginDestroy(); // 通知 GC 清理对象
	}

	UInvSys_BaseEquipmentObject* EquipObj = GetInventoryObject<UInvSys_BaseEquipmentObject>(SlotTag);
	if (EquipObj != nullptr && EquipObj->GetEquipItemInstance() == nullptr)
	{
		TargetItemInstance->SetSlotTag(SlotTag);
		TargetItemInstance->SetInventoryComponent(this);
		
		UE_LOG(LogInventorySystem, Log, TEXT("正在装备物品"))

		EquipObj->EquipInventoryItem(TargetItemInstance);
		// 检查目标内部是否存在其他的物品
		if (TargetItemInstance->MyInstances.Num() > 0)
		{
			UInvSys_BaseEquipContainerObject* ContainerObject = GetInventoryObject<UInvSys_BaseEquipContainerObject>(SlotTag);
			check(ContainerObject)
			UE_LOG(LogInventorySystem, Log, TEXT("物品内部包含了[%d]个其他物品，现在正在对其内部物品进行转移。"), TargetItemInstance->MyInstances.Num())
			ContainerObject->AddItemInstances(TargetItemInstance->MyInstances);
			TargetItemInstance->MyInstances.Empty();
		}
		OnEquipItemInstance(TargetItemInstance);
	}
}

void UInvSys_InventoryComponent::UnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance)
	{
		FGameplayTag EquipSlotTag = InItemInstance->GetSlotTag();
		UInvSys_BaseEquipmentObject* EquipObj = GetInventoryObject<UInvSys_BaseEquipmentObject>(EquipSlotTag);
		if (EquipObj != nullptr && EquipObj->GetEquipItemInstance() == InItemInstance)
		{
			EquipObj->UnEquipInventoryItem();
			OnUnEquipItemInstance(InItemInstance);
		}
	}
}

bool UInvSys_InventoryComponent::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("正在删除目标：%s"), *InItemInstance->GetName());
		FGameplayTag EquipSlotTag = InItemInstance->GetSlotTag();
		// 这里的InvObject为最最基础的Object
		if (UInvSys_BaseInventoryObject* LOCAL_InvObj = GetInventoryObject(EquipSlotTag))
		{
			return LOCAL_InvObj->RemoveItemInstance(InItemInstance);
		}
	}
	return false;
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

bool UInvSys_InventoryComponent::IsContainsInventoryItem(const FName ItemUniqueID)
{
	for (UInvSys_BaseInventoryObject* InventoryObject : InventoryObjectList)
	{
		if (InventoryObject->ContainsItem(ItemUniqueID))
		{
			return true;
		}
	}
	return false;
}

bool UInvSys_InventoryComponent::IsLocalController() const
{
	ensure(OwningPlayer);
	return OwningPlayer ? OwningPlayer->IsLocalController() : false;
}

void UInvSys_InventoryComponent::ConstructInventoryObjects()
{
	// 权威服务器更新库存对象列表与映射关系
	if (InventoryContentMapping == nullptr)
	{
#if WITH_EDITOR
		FNotificationInfo NotificationInfo(FText::FromString("Inventory Content Mapping Is NULL."));
		NotificationInfo.ExpireDuration = 5.f;
		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
#endif
		UE_LOG(LogInventorySystem, Error, TEXT("Inventory Content Mapping Is NULL."));
		return;
	}
	if (HasAuthority())
	{
		InventoryObjectList.Empty();
		InventoryObjectList.Reserve(InventoryContentMapping->InventoryContentList.Num());
		
		UE_LOG(LogInventorySystem, Log, TEXT("== 正在构建库存对象 [%s:%s] =="),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwner()->GetName());
		for (UInvSys_PreEditInventoryObject* PreEditInventoryObject : InventoryContentMapping->InventoryContentList)
		{
			if (PreEditInventoryObject == nullptr)
			{
				UE_LOG(LogInventorySystem, Warning, TEXT("库存对象内容映射 [InventoryContentMapping] 中存在空值."));
				continue;
			}
			UInvSys_BaseInventoryObject* InventoryObject = PreEditInventoryObject->ConstructInventoryObject(this);
			if (InventoryObject == nullptr)
			{
				UE_LOG(LogInventorySystem, Warning, TEXT("预设 [%s] 构建的库存对象为空."), *PreEditInventoryObject->GetName());
				continue;
			}
			InventoryObjectList.Add(InventoryObject);
			//AddReplicatedSubObject(InventoryObject, COND_None);//若出现失序问题，请取消这段代码的注释。
		}
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnRep_InventoryObjectList();
		}
	}
}

/*void UInvSys_InventoryComponent::Server_TryDragItemInstance_Implementation(
	UInvSys_InventoryItemInstance* InItemInstance)
{
	bool LOCAL_IsSuccess = TryDragItemInstance(InItemInstance);
	if (LOCAL_IsSuccess == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("[Server] 服务器尝试拽起目标物品失败，可能是目标物品为NULL或服务器未生成该物品。"))
	}
}

void UInvSys_InventoryComponent::Server_CancelDragItemInstance_Implementation(
	UInvSys_InventoryItemInstance* InItemInstance)
{
	CancelDragItemInstance();
}*/

void UInvSys_InventoryComponent::Server_EquipItemInstance_Implementation(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag)
{
	check(InvComp);
	if (InvComp)
	{
		InvComp->EquipItemInstance(InItemInstance, SlotTag);
	}
}

void UInvSys_InventoryComponent::Server_EquipItemDefinition_Implementation(
	UInvSys_InventoryComponent* InvComp, TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, FGameplayTag SlotTag)
{
	check(InvComp);
	if (InvComp)
	{
		InvComp->EquipItemDefinition(ItemDef, SlotTag);
	}
}

void UInvSys_InventoryComponent::Server_TryDragItemInstance_Implementation(UInvSys_InventoryComponent* InvComp,
	UInvSys_InventoryItemInstance* InItemInstance)
{
	check(InvComp)
	if (InvComp)
	{
		InvComp->TryDragItemInstance(InItemInstance);
	}
}

void UInvSys_InventoryComponent::Server_CancelDragItemInstance_Implementation(UInvSys_InventoryComponent* InvComp)
{
	check(InvComp)
	InvComp->CancelDragItemInstance();
}

void UInvSys_InventoryComponent::Server_RestoreItemInstance_Implementation(UInvSys_InventoryComponent* InvComp,
	UInvSys_InventoryItemInstance* InItemInstance)
{
	check(InvComp)
	if (InvComp)
	{
		InvComp->RestoreItemInstance(InItemInstance);
	}
}

void UInvSys_InventoryComponent::SetDraggingWidget(UUserWidget* NewDraggingWidget)
{
	DraggingWidget = NewDraggingWidget;
}

UInvSys_BaseInventoryObject* UInvSys_InventoryComponent::GetInventoryObject(FGameplayTag Tag,
                                                                            TSubclassOf<UInvSys_BaseInventoryObject> OutClass) const
{
	return GetInventoryObject<UInvSys_BaseInventoryObject>(Tag);
}

UInvSys_PreEditInventoryObject* UInvSys_InventoryComponent::GetPreEditInventoryObject(int32 Index) const
{
	if (InventoryContentMapping && InventoryContentMapping->InventoryContentList.IsValidIndex(Index))
	{
		return InventoryContentMapping->InventoryContentList[Index];
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("GetPreEditInventoryObject 传入的 Index 无效。"))
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
		
		// 让库存子对象复制自己的属性之前，复制子对象。
		//UActorChannel::SetCurrentSubObjectOwner(this);
		WroteSomething |= InventoryObject->ReplicateSubobjects(Channel, Bunch, RepFlags);

		//UActorChannel::SetCurrentSubObjectOwner(GetOwner());
		WroteSomething |= Channel->ReplicateSubobject(InventoryObject, *Bunch, *RepFlags);
	}

	// if (DraggingItemInstance && IsValid(DraggingItemInstance))
	// {
	// 	WroteSomething |= Channel->ReplicateSubobject(DraggingItemInstance, *Bunch, *RepFlags);
	// }
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
		UE_LOG(LogInventorySystem, Log, TEXT("初始化复制"))
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
				UE_LOG(LogInventorySystem, Log, TEXT("初始化复制 ===》 容器项"))
				AddReplicatedSubObject(Entry.Instance);
			}
		}
	}*/
}

void UInvSys_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_InventoryComponent, InventoryObjectList);
	// DOREPLIFETIME(UInvSys_InventoryComponent, DraggingItemInstance);
}

void UInvSys_InventoryComponent::OnRep_InventoryObjectList()
{
	UE_LOG(LogInventorySystem, Log, TEXT("[%d] OnRep 初始化库存对象"), InventoryObjectList.Num())
	for (int i = 0; i < InventoryObjectList.Num(); ++i)
	{
		check(InventoryObjectList[i]);
		InventoryObjectList[i]->OnConstructInventoryObject(this, GetPreEditInventoryObject(i));
		InventoryObjectMap.Add(InventoryObjectList[i]->GetSlotTag(), InventoryObjectList[i]);
	}
	
	OnConstructInventoryObjects();

	//所有对象构建完成后自动移除数据资产
	/*if (InventoryContentMapping)
	{
		FPrimaryAssetId PrimaryAssetId = InventoryContentMapping->GetPrimaryAssetId();
		if (PrimaryAssetId.IsValid())
		{
			int32 Count = UAssetManager::Get().UnloadPrimaryAsset(PrimaryAssetId);
			UE_LOG(LogInventorySystem, Log, TEXT("卸载库存数据资产，当前引用数为 = %d"), Count);
		}
		InventoryContentMapping = nullptr;
	}*/
}

UInvSys_InventoryLayoutWidget* UInvSys_InventoryComponent::CreateDisplayWidget(APlayerController* NewPlayerController)
{
	if (bDisplayWidgetIsValid)
	{
		return LayoutWidget;
	}
	if (NewPlayerController && NewPlayerController->IsLocalController())
	{
		bDisplayWidgetIsValid = true;
		OwningPlayer = NewPlayerController;
	
		UE_LOG(LogInventorySystem, Log, TEXT("== 正在创建显示控件 [%s:%s] =="),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwner()->GetName())

		// 创建布局控件后，收集所有的 TagSlot 供后续控件插入正确位置。
		LayoutWidget = CreateWidget<UInvSys_InventoryLayoutWidget>(NewPlayerController, LayoutWidgetClass);
		check(LayoutWidget)
		LayoutWidget->CollectAllTagSlots();
		LayoutWidget->SetInventoryComponent(this);
		
		for (int i = 0; i < InventoryObjectList.Num(); ++i)
		{
			if (InventoryObjectList[i] == nullptr || InventoryObjectList[i]->IsA(UInvSys_BaseEquipmentObject::StaticClass()) == false)
			{
				checkNoEntry();
				continue;
			}

			UInvSys_BaseEquipmentObject* EquipObj = Cast<UInvSys_BaseEquipmentObject>(InventoryObjectList[i]);
			check(EquipObj)
			// 将库存对象的控件插入对应位置的插槽。
			UInvSys_EquipSlotWidget* EquipSlotWidget = EquipObj->CreateDisplayWidget(NewPlayerController);
			FGameplayTag SlotTag = EquipObj->GetSlotTag();
			UInvSys_TagSlot* TagSlot = LayoutWidget->FindTagSlot(SlotTag); // 根据槽标签，查找对应的槽控件。
			if (TagSlot)
			{
				TagSlot->AddChild(EquipSlotWidget);
			}
		}
	}
	return LayoutWidget;
}

void UInvSys_InventoryComponent::Client_TryRefreshInventoryObject_Implementation()
{
	checkNoEntry();
	for (UInvSys_BaseInventoryObject* InvObj : InventoryObjectList)
	{
		
		InvObj->RefreshInventoryObject("客户端数据与服务器数据不匹配，请求刷新客户端显示效果。");
	}
}


// void UInvSys_InventoryComponent::OnRep_DraggingItemInstance()
// {
	// Server端DraggingWidget一定是为空的，且DraggingInstance大概率是存在的
	// Client端则不会为空
	// 修正拖拽的物品实例
	/*if (IsValid(DraggingWidget))
	{
		// 客户端同步更新DraggingItem
		if (IsValid(DraggingItemInstance))
		{
			NativeOnDraggingItemInstance(DraggingItemInstance);
		}
		else
		{
			NativeOnCancelDragItemInstance();
		}
	}
	else if(IsValid(DraggingItemInstance))
	{
		// 除非用户手动调用丢弃至世界，否则不会执行其他操作。
		// NativeOnDiscardItemInstance(DraggingItemInstance); //丢弃物品至世界。
	}*/
// }
