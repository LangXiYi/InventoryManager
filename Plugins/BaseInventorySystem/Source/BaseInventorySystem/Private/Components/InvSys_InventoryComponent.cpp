// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InvSys_InventoryComponent.h"

#include "BaseInventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
#include "Data/InvSys_InventoryContentMapping.h"
#include "Engine/ActorChannel.h"
#include "Engine/AssetManager.h"
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
	check(bReplicateUsingRegisteredSubObjectList == false);
	//bReplicateUsingRegisteredSubObjectList = true; // 不推荐使用，会出现子对象与FastArray的属性同步失序的问题。
}

void UInvSys_InventoryComponent::AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem)
{
	/*if (InventoryObjectMap_DEPRECATED.Contains(NewItem.SlotName) == false)
	{
		UE_LOG(LogInventorySystem, Log, TEXT("%s 必须在 InventoryObjectMap 中存在的。"), *NewItem.SlotName.ToString());
		return;
	}

	if (InventoryObjectMap_DEPRECATED[NewItem.SlotName]->IsA(UInvSys_BaseEquipmentObject::StaticClass()))
	{
		UE_LOG(LogInventorySystem, Log, TEXT("[%s:%s] EquipSlot add item [%s] to [%s]."),
			HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwner()->GetName(),
			*NewItem.ItemID.ToString(), *NewItem.SlotName.ToString());

		UInvSys_BaseEquipmentObject* EquipmentObj = Cast<UInvSys_BaseEquipmentObject>(InventoryObjectMap_DEPRECATED[NewItem.SlotName]);
		EquipmentObj->AddInventoryItemToEquipSlot_DEPRECATED(NewItem);
	}*/
}

void UInvSys_InventoryComponent::EquipInventoryItem(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDefinition, FGameplayTag SlotTag)
{
	if (HasAuthority())
	{
		UInvSys_BaseEquipmentObject* EquipObj = GetInventoryObject<UInvSys_BaseEquipmentObject>(SlotTag);
		if (EquipObj != nullptr)
		{
			EquipObj->EquipInventoryItem(ItemDefinition);
		}
	}
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

// Called when the game starts
void UInvSys_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (HasAuthority())
	{
		ConstructInventoryObjects();
	}
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

	DOREPLIFETIME_CONDITION(UInvSys_InventoryComponent, InventoryObjectList, COND_None);
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
			UInvSys_EquipSlotWidget* EquipSlotWidget = EquipObj->CreateEquipSlotWidget(NewPlayerController);
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

