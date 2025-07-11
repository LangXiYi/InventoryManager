// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseEquipContainerObject.h"

#include "BaseInventorySystem.h"
#include "Data/InvSys_ItemFragment_ContainerLayout.h"
#include "Blueprint/UserWidget.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/InvSys_EquipContainerSlotWidget.h"
#include "Widgets/InvSys_EquipSlotWidget.h"


UInvSys_BaseEquipContainerObject::UInvSys_BaseEquipContainerObject()
	:ContainerList(this)
{
	if (ContainerList.OnContainerEntryAddedDelegate().IsBound() == false)
	{
		ContainerList.OnContainerEntryAddedDelegate().BindUObject(this,
			&UInvSys_BaseEquipContainerObject::NativeOnContainerEntryAdded);
	}

	if (ContainerList.OnContainerEntryRemoveDelegate().IsBound() == false)
	{
		ContainerList.OnContainerEntryRemoveDelegate().BindUObject(this,
			&UInvSys_BaseEquipContainerObject::NativeOnContainerEntryRemove);
	}
	/*ContainerList.OnInventoryStackChangeDelegate().AddUObject(this,
		&UInvSys_BaseEquipContainerObject::NativeOnInventoryStackChange);*/
}

bool UInvSys_BaseEquipContainerObject::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                           FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (FInvSys_ContainerEntry Entry : ContainerList.Entries)
	{
		UInvSys_InventoryItemInstance* Instance = Entry.Instance;
		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
		// UE_LOG(LogInventorySystem, Error, TEXT("复制 Fast Array 的子对象 = %s]"), *Instance->GetName());
	}
	return WroteSomething;
}

void UInvSys_BaseEquipContainerObject::NativeOnInventoryStackChange(FInvSys_InventoryStackChangeMessage ChangeInfo)
{
	OnInventoryStackChange(ChangeInfo); // 监听 Stack Count 变化
}

void UInvSys_BaseEquipContainerObject::NativeOnContainerEntryAdded(const FInvSys_ContainerEntry& Entry, bool bIsInit)
{
	if (Entry.Instance)
	{
		Entry.Instance->OnInventoryStackChangeDelegate().BindUObject(this, &UInvSys_BaseEquipContainerObject::NativeOnInventoryStackChange);
		Entry.Instance->BroadcastStackChangeMessage(0, Entry.Instance->GetStackCount());
	}
	OnContainerEntryAdded(Entry, bIsInit);// 监听 Item Instance 变化
}

void UInvSys_BaseEquipContainerObject::NativeOnContainerEntryRemove(const FInvSys_ContainerEntry& Entry, bool bIsInit)
{
	if (Entry.Instance)
	{
		Entry.Instance->BroadcastStackChangeMessage(Entry.Instance->GetStackCount(), 0);
		Entry.Instance->OnInventoryStackChangeDelegate().Unbind();
	}
	OnContainerEntryRemove(Entry, bIsInit); // 移除监听 Item Instance 变化
}

void UInvSys_BaseEquipContainerObject::OnConstructInventoryObject(UInvSys_InventoryComponent* NewInvComp,
                                                                  UObject* PreEditPayLoad)
{
	Super::OnConstructInventoryObject(NewInvComp, PreEditPayLoad);
}

UInvSys_EquipSlotWidget* UInvSys_BaseEquipContainerObject::CreateDisplayWidget(APlayerController* PC)
{
	UInvSys_EquipSlotWidget* TempEquipSlotWidget = Super::CreateDisplayWidget(PC);

	return TempEquipSlotWidget;
}

void UInvSys_BaseEquipContainerObject::TryRefreshEquipSlot(const FString& Reason)
{
	Super::TryRefreshEquipSlot(Reason);

	// 根据EquipItemInstance的值判断是移除布局还是添加布局
	/*if (EquipSlotWidget == nullptr)
	{
		return;
	}
	
	if (EquipItemInstance)
	{
		auto Fragment = EquipItemInstance->FindFragmentByClass<UInvSys_ItemFragment_ContainerLayout>();
		if (Fragment)
		{
			ContainerLayout = CreateWidget<UInvSys_InventoryWidget>(EquipSlotWidget, Fragment->ContainerLayout);
			if (EquipSlotWidget->IsA(UInvSys_EquipContainerSlotWidget::StaticClass()))
			{
				UInvSys_EquipContainerSlotWidget* TempContainerSlotWidget = Cast<UInvSys_EquipContainerSlotWidget>(EquipSlotWidget);
				check(TempContainerSlotWidget);
				TempContainerSlotWidget->AddContainerLayout(ContainerLayout);
				TryRefreshContainerItems();
			}
		}
		
	}
	else
	{
		ContainerLayout->RemoveFromParent();
		ContainerLayout = nullptr;
		// TODO::删除布局
		// EquipSlotWidget->UnEquipItemInstance();
	}*/
}

void UInvSys_BaseEquipContainerObject::TryRefreshContainerItems()
{
	UE_LOG(LogInventorySystem, Log, TEXT("正在刷新容器内所有物品！"))
	for (const FInvSys_ContainerEntry& Entry : ContainerList.Entries)
	{
		ContainerList.BroadcastRemoveEntryMessage(Entry, true);
	}
	for (const FInvSys_ContainerEntry& Entry : ContainerList.Entries)
	{
		ContainerList.BroadcastAddEntryMessage(Entry, true);
	}
}

bool UInvSys_BaseEquipContainerObject::AddItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr) return false;
	
	ItemInstance->SetSlotTag(EquipSlotTag);
	ItemInstance->SetInventoryComponent(InventoryComponent);
	
	return ContainerList.AddEntry(ItemInstance);
}

bool UInvSys_BaseEquipContainerObject::AddItemInstances(TArray<UInvSys_InventoryItemInstance*> ItemInstances)
{
	bool bResult = true;
	for (UInvSys_InventoryItemInstance* ItemInstance : ItemInstances)
	{
		bResult &= AddItemInstance(ItemInstance);
	}
	return bResult;
}

bool UInvSys_BaseEquipContainerObject::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	UE_LOG(LogInventorySystem, Log, TEXT("正在删除容器内的所有物品"))
	bool LOCAL_IsRemoveEquipContainer = Super::RemoveItemInstance(InItemInstance);
	if (LOCAL_IsRemoveEquipContainer)
	{
		// 移除的是容器本体，需要将所有的物品打包转移。
		InItemInstance->MyInstances.Empty();
		InItemInstance->MyInstances.Reserve(ContainerList.Entries.Num());
		for (UInvSys_InventoryItemInstance* ContainerEntry : ContainerList.GetAllItems())
		{
			// 将所有物品转移至容器的物品实例中保存。
			InItemInstance->MyInstances.Add(ContainerEntry);
			ContainerList.RemoveEntry(ContainerEntry);
		}
		return true;
	}
	return ContainerList.RemoveEntry(InItemInstance);
}

bool UInvSys_BaseEquipContainerObject::RestoreItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	bool bResult = false;
	check(InItemInstance);
	if (InItemInstance)
	{
		bResult |= Super::RestoreItemInstance(InItemInstance);
		if (InItemInstance->MyInstances.Num() > 0)
		{
			//如果内部存在其他物品则表明该物品是一个装备容器，那么就需要将内部物品全部转移进来。
			bResult |= AddItemInstances(InItemInstance->MyInstances);
		}
		else if(bResult == false)
		{
			//内部没有其他物品，且物品不会被装备，说明它只是一个普通的物品
			bResult |= AddItemInstance(InItemInstance);
		}
	}
	return bResult;
}

bool UInvSys_BaseEquipContainerObject::UpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance, int32 NewStackCount)
{
	/*if (ItemInstance == nullptr) return false;
	return ContainerList.UpdateEntryStackCount(ItemInstance, NewStackCount);*/
	return false;
}

void UInvSys_BaseEquipContainerObject::NativeOnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	Super::NativeOnEquipItemInstance(InItemInstance);
	if (EquipSlotWidget && InItemInstance)
	{
		auto Fragment = InItemInstance->FindFragmentByClass<UInvSys_ItemFragment_ContainerLayout>();
		if (Fragment)
		{
			ContainerLayout = CreateWidget<UInvSys_InventoryWidget>(EquipSlotWidget, Fragment->ContainerLayout);
			ContainerLayout->SetInventoryComponent(InventoryComponent);
			if (EquipSlotWidget->IsA(UInvSys_EquipContainerSlotWidget::StaticClass()))
			{
				UInvSys_EquipContainerSlotWidget* TempContainerSlotWidget = Cast<UInvSys_EquipContainerSlotWidget>(EquipSlotWidget);
				check(TempContainerSlotWidget);
				TempContainerSlotWidget->AddContainerLayout(ContainerLayout);
				TryRefreshContainerItems();
			}
		}
	}
}

void UInvSys_BaseEquipContainerObject::NativeOnUnEquipItemInstance()
{
	Super::NativeOnUnEquipItemInstance();
	if (EquipSlotWidget && ContainerLayout)
	{
		ContainerLayout->RemoveFromParent();
		ContainerLayout = nullptr;
		// TODO::删除布局
		// EquipSlotWidget->UnEquipItemInstance();
	}
}

void UInvSys_BaseEquipContainerObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_BaseEquipContainerObject, ContainerList);
}

const UInvSys_InventoryItemInstance* UInvSys_BaseEquipContainerObject::FindItemInstance(FGuid ItemUniqueID) const
{
	if (ContainerEntryMap.Contains(ItemUniqueID))
	{
		return ContainerEntryMap[ItemUniqueID].Instance;
	}
	return nullptr;
}

bool UInvSys_BaseEquipContainerObject::ContainsItem(FName UniqueID)
{
	return Super::ContainsItem(UniqueID)/* || ContainerItems.Contains(UniqueID)*/;
}

void UInvSys_BaseEquipContainerObject::CopyPropertyFromPreEdit(UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(PreEditPayLoad);

}
