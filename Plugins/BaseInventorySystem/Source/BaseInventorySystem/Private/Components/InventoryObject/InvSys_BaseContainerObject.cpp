// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseContainerObject.h"

#include "InvSys_InventorySystemConfig.h"
#include "Net/UnrealNetwork.h"


UInvSys_BaseContainerObject::UInvSys_BaseContainerObject()
{
}

void UInvSys_BaseContainerObject::InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent,
	UObject* PreEditPayLoad)
{
	Super::InitInventoryObject(NewInventoryComponent, PreEditPayLoad);
	TryRefreshContainerItems("InitInventoryObjects() ===> TryRefreshContainerItems()");
}

void UInvSys_BaseContainerObject::AddDataToRep_AddedInventoryItems(FName ItemUniqueID)
{
	if (HasAuthority())
	{
		bIsWait_Pending_AddedInventoryItems = true;
		Pending_AddedInventoryItems.Add(ItemUniqueID);
		UE_LOG(LogInventorySystem, Log, TEXT("[%s]Added Inventory Items."), *GetOwner()->GetName())
		TryRepInventoryItems_Add();
	}
}

void UInvSys_BaseContainerObject::AddDataToRep_RemovedInventoryItems(FName ItemUniqueID)
{
	if (HasAuthority())
	{
		bIsWait_Pending_RemovedInventoryItems = true;
		Pending_RemovedInventoryItems.Add(ItemUniqueID);
		TryRepInventoryItems_Remove();
	}
}

void UInvSys_BaseContainerObject::AddDataToRep_ChangedInventoryItems(FName OldItemUniqueID)
{
	if (HasAuthority())
	{
		bIsWait_Pending_ChangedInventoryItems = true;
		Pending_ChangedInventoryItems.Add(OldItemUniqueID);
		TryRepInventoryItems_Change();
	}
}

void UInvSys_BaseContainerObject::TryRefreshContainerItems(const FString& Reason)
{
	if (Reason != "") UE_LOG(LogInventorySystem, Log, TEXT("[%s]"), *Reason);
}

void UInvSys_BaseContainerObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_BaseContainerObject, ChangedInventoryItems);
	DOREPLIFETIME(UInvSys_BaseContainerObject, AddedInventoryItems);
	DOREPLIFETIME(UInvSys_BaseContainerObject, RemovedInventoryItems);
}
void UInvSys_BaseContainerObject::TryRepInventoryItems_Add()
{
	if (HasAuthority() == false || AddTimerHandle.IsValid()) return;
	// 第一批数据，直接发送无延迟，后续数据等待 N 秒后，将 N 秒内增加的数据合批发送。
	// 优点：保证正常玩家使用无延迟，对高频数据会自动合批发送，减少发送频率。
	AActor* OwningActor = GetOwner();
	check(OwningActor)
	if (OwningActor == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("OwningActor is nullptr."))
		return;
	}
	AddedInventoryItems = Pending_AddedInventoryItems;
	Pending_AddedInventoryItems.Empty();
	OwningActor->ForceNetUpdate();
	
	if (GetNetMode() != NM_DedicatedServer && IsLocallyControlled())
		OnRep_AddedInventoryItems();

	// 发送数据后等待 N 秒后清除旧数据，若在等待期间缓存数据增加，则清除完成后再次调用本函数，将数据发送至客户端。
	GetWorld()->GetTimerManager().SetTimer(AddTimerHandle, [this]() {
		AddedInventoryItems.Empty();
		AddTimerHandle.Invalidate();
		if (!Pending_AddedInventoryItems.IsEmpty())
		{
			UE_LOG(LogInventorySystem, Log, TEXT("Pending_AddedInventoryItems 中存在新的数据，需要先进行强制更新。"))
			TryRepInventoryItems_Add(); // 存在新的需要更新的数据，强制继续更新。
		}
	}, GetServerWaitBatchTime(), false);
}

void UInvSys_BaseContainerObject::TryRepInventoryItems_Remove()
{
	if (HasAuthority() == false || RemoveTimerHandle.IsValid()) return;
	// 第一批数据，直接发送无延迟，后续数据等待 N 秒后，将 N 秒内增加的数据合批发送。
	// 优点：保证正常玩家使用无延迟，对高频数据会自动合批发送，减少发送频率。
	AActor* OwningActor = GetOwner();
	check(OwningActor)
	if (OwningActor == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("OwningActor is nullptr."))
		return;
	}
	RemovedInventoryItems = Pending_RemovedInventoryItems;
	Pending_RemovedInventoryItems.Empty();
	OwningActor->ForceNetUpdate();

	if (GetNetMode() != NM_DedicatedServer && IsLocallyControlled())
		OnRep_RemovedInventoryItems();

	// 发送数据后等待 N 秒后清除旧数据，若在等待期间缓存数据增加，则清除完成后再次调用本函数，将数据发送至客户端。
	GetWorld()->GetTimerManager().SetTimer(RemoveTimerHandle, [this]() {
		RemovedInventoryItems.Empty();
		RemoveTimerHandle.Invalidate();
		if (!Pending_RemovedInventoryItems.IsEmpty())
		{
			UE_LOG(LogInventorySystem, Log, TEXT("Pending_AddedInventoryItems 中存在新的数据，需要先进行强制更新。"))
			TryRepInventoryItems_Remove(); // 存在新的需要更新的数据，强制继续更新。
		}
	}, GetServerWaitBatchTime(), false);
}

void UInvSys_BaseContainerObject::TryRepInventoryItems_Change()
{
	if (HasAuthority() == false || ChangeTimerHandle.IsValid()) return;
	// 第一批数据，直接发送无延迟，后续数据等待 N 秒后，将 N 秒内增加的数据合批发送。
	// 优点：保证正常玩家使用无延迟，对高频数据会自动合批发送，减少发送频率。
	AActor* OwningActor = GetOwner();
	check(OwningActor)
	if (OwningActor == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("OwningActor is nullptr."))
		return;
	}
	ChangedInventoryItems = Pending_ChangedInventoryItems;
	Pending_ChangedInventoryItems.Empty();
	OwningActor->ForceNetUpdate();

	if (GetNetMode() != NM_DedicatedServer && IsLocallyControlled())
		OnRep_ChangedInventoryItems();

	// 发送数据后等待 N 秒后清除旧数据，若在等待期间缓存数据增加，则清除完成后再次调用本函数，将数据发送至客户端。
	GetWorld()->GetTimerManager().SetTimer(ChangeTimerHandle, [this]() {
		ChangedInventoryItems.Empty();
		ChangeTimerHandle.Invalidate();
		if (!Pending_ChangedInventoryItems.IsEmpty())
		{
			UE_LOG(LogInventorySystem, Log, TEXT("Pending_AddedInventoryItems 中存在新的数据，需要先进行强制更新。"))
			TryRepInventoryItems_Change(); // 存在新的需要更新的数据，强制继续更新。
		}
	}, GetServerWaitBatchTime(), false);
}

void UInvSys_BaseContainerObject::OnRep_ChangedInventoryItems()
{
	OnUpdatedContainerItems(ChangedInventoryItems);
}

void UInvSys_BaseContainerObject::OnRep_AddedInventoryItems()
{
	OnAddedContainerItems(AddedInventoryItems);
}

void UInvSys_BaseContainerObject::OnRep_RemovedInventoryItems()
{
	OnRemovedContainerItems(RemovedInventoryItems);
}