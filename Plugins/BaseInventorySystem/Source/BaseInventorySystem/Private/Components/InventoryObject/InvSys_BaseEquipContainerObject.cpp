// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseEquipContainerObject.h"

#include "InvSys_InventorySystemConfig.h"
#include "Net/UnrealNetwork.h"

UInvSys_BaseEquipContainerObject::UInvSys_BaseEquipContainerObject()
{

}

void UInvSys_BaseEquipContainerObject::RefreshInventoryObject(const FString& Reason)
{
	Super::RefreshInventoryObject(Reason);
	TryRefreshContainerItems();
}

void UInvSys_BaseEquipContainerObject::RecordItemOperationByAdd(FName ItemUniqueID)
{
	if (HasAuthority())
	{
		Pending_AddedInventoryItems.Add(ItemUniqueID);
		ContainerItems.Add(ItemUniqueID);
		TryApplyAddOperations();
	}
}

void UInvSys_BaseEquipContainerObject::RecordItemOperationByRemove(FName ItemUniqueID)
{
	if (HasAuthority())
	{
		Pending_RemovedInventoryItems.Add(ItemUniqueID);
		ContainerItems.Remove(ItemUniqueID);
		TryApplyRemoveOperations();
	}
}

void UInvSys_BaseEquipContainerObject::RecordItemOperationByUpdate(FName ItemUniqueID)
{
	if (HasAuthority())
	{
		Pending_ChangedInventoryItems.Add(ItemUniqueID);
		TryApplyUpdateOperations();
	}
}

void UInvSys_BaseEquipContainerObject::TryRefreshOccupant(const FString& Reason)
{
	Super::TryRefreshOccupant(Reason);
}

void UInvSys_BaseEquipContainerObject::TryRefreshContainerItems(const FString& Reason)
{
	if (Reason != "") UE_LOG(LogInventorySystem, Log, TEXT("[%s]"), *Reason);
}

void UInvSys_BaseEquipContainerObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_BaseEquipContainerObject, ChangedInventoryItems);
	DOREPLIFETIME(UInvSys_BaseEquipContainerObject, AddedInventoryItems);
	DOREPLIFETIME(UInvSys_BaseEquipContainerObject, RemovedInventoryItems);
}

void UInvSys_BaseEquipContainerObject::TryApplyAddOperations()
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
	AddedInventoryItems.Append(Pending_AddedInventoryItems.Array());
	Pending_AddedInventoryItems.Empty();
	OwningActor->ForceNetUpdate();
	
	if (GetNetMode() != NM_DedicatedServer)
		OnRep_AddedInventoryItems();

	// 发送数据后等待 N 秒后清除旧数据，若在等待期间缓存数据增加，则清除完成后再次调用本函数，将数据发送至客户端。
	GetWorld()->GetTimerManager().SetTimer(AddTimerHandle, [this]() {
		AddedInventoryItems.Empty();
		AddTimerHandle.Invalidate();
		if (!Pending_AddedInventoryItems.IsEmpty())
		{
			// UE_LOG(LogInventorySystem, Log, TEXT("Pending_AddedInventoryItems 中存在新的数据，需要先进行强制更新。"))
			TryApplyAddOperations(); // 存在新的需要更新的数据，强制继续更新。
		}
	}, GetServerWaitBatchTime(), false);
}

void UInvSys_BaseEquipContainerObject::TryApplyRemoveOperations()
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
	RemovedInventoryItems.Append(Pending_RemovedInventoryItems.Array());
	Pending_RemovedInventoryItems.Empty();
	UE_LOG(LogInventorySystem, Log, TEXT("Net update to client."))
	OwningActor->ForceNetUpdate();

	if (GetNetMode() != NM_DedicatedServer)
		OnRep_RemovedInventoryItems();

	// 发送数据后等待 N 秒后清除旧数据，若在等待期间缓存数据增加，则清除完成后再次调用本函数，将数据发送至客户端。
	GetWorld()->GetTimerManager().SetTimer(RemoveTimerHandle, [this]() {
		RemovedInventoryItems.Empty();
		RemoveTimerHandle.Invalidate();
		if (!Pending_RemovedInventoryItems.IsEmpty())
		{
			// UE_LOG(LogInventorySystem, Log, TEXT("Pending_AddedInventoryItems 中存在新的数据，需要先进行强制更新。"))
			TryApplyRemoveOperations(); // 存在新的需要更新的数据，强制继续更新。
		}
	}, GetServerWaitBatchTime(), false);
}

void UInvSys_BaseEquipContainerObject::TryApplyUpdateOperations()
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
	ChangedInventoryItems.Append(Pending_ChangedInventoryItems.Array());
	Pending_ChangedInventoryItems.Empty();
	OwningActor->ForceNetUpdate();

	if (GetNetMode() != NM_DedicatedServer)
		OnRep_ChangedInventoryItems();

	// 发送数据后等待 N 秒后清除旧数据，若在等待期间缓存数据增加，则清除完成后再次调用本函数，将数据发送至客户端。
	GetWorld()->GetTimerManager().SetTimer(ChangeTimerHandle, [this]() {
		ChangedInventoryItems.Empty();
		ChangeTimerHandle.Invalidate();
		if (!Pending_ChangedInventoryItems.IsEmpty())
		{
			// UE_LOG(LogInventorySystem, Log, TEXT("Pending_AddedInventoryItems 中存在新的数据，需要先进行强制更新。"))
			TryApplyUpdateOperations(); // 存在新的需要更新的数据，强制继续更新。
		}
	}, GetServerWaitBatchTime(), false);
}

bool UInvSys_BaseEquipContainerObject::ContainsItem(FName UniqueID)
{
	return Super::ContainsItem(UniqueID) || ContainerItems.Contains(UniqueID);
}

void UInvSys_BaseEquipContainerObject::OnRep_AddedInventoryItems()
{
	OnAddedContainerItems(AddedInventoryItems);
}

void UInvSys_BaseEquipContainerObject::OnRep_RemovedInventoryItems()
{
	OnRemovedContainerItems(RemovedInventoryItems);
}

void UInvSys_BaseEquipContainerObject::OnRep_ChangedInventoryItems()
{
	OnUpdatedContainerItems(ChangedInventoryItems);
}
