// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_BaseContainerObject.h"
#include "InvSys_BaseEquipmentObject.h"
#include "InvSys_BaseEquipContainerObject.generated.h"

enum class EInventoryUpdateType : uint8
{
	Add, Remove, Change		
};

/**
 *
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_BaseEquipContainerObject : public UInvSys_BaseEquipmentObject
{
	GENERATED_BODY()

public:
	/** [Server] 添加物品，并将操作记录至复制列表，等待一段时间后，将所有操作批量发送给客户端并清除操作列表。 */
	void AddDataToRep_AddedInventoryItems(FName ItemUniqueID);
	/** [Server] 删除物品，并将操作记录至复制列表，等待一段时间后，将所有操作批量发送给客户端并清除操作列表。 */
	void AddDataToRep_RemovedInventoryItems(FName ItemUniqueID);
	/** [Server] 修改物品，并将操作记录至操作列表，等待一段时间后，将所有操作批量发送给客户端并清除操作列表。 */
	void AddDataToRep_ChangedInventoryItems(FName ItemUniqueID);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** [Client] 处理服务器批量返回的操作记录 */
	virtual void OnAddedContainerItems(const TArray<FName>& InAddedItems) {}
	/** [Client] 处理服务器批量返回的操作记录 */
	virtual void OnRemovedContainerItems(const TArray<FName>& InRemovedItems) {}
	/** [Client] 处理服务器批量返回的操作记录 */
	virtual void OnUpdatedContainerItems(const TArray<FName>& InChangedItems) {}

private:
	void TryRepInventoryItems_Add();
	void TryRepInventoryItems_Remove();
	void TryRepInventoryItems_Change();
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_AddedInventoryItems)
	TArray<FName> AddedInventoryItems;
	TArray<FName> Pending_AddedInventoryItems;
	bool bIsWait_Pending_AddedInventoryItems = false;
	UFUNCTION()
	void OnRep_AddedInventoryItems();

	UPROPERTY(ReplicatedUsing = OnRep_RemovedInventoryItems)
	TArray<FName> RemovedInventoryItems;
	TArray<FName> Pending_RemovedInventoryItems;
	bool bIsWait_Pending_RemovedInventoryItems = false;
	UFUNCTION()
	void OnRep_RemovedInventoryItems();

	UPROPERTY(ReplicatedUsing = OnRep_ChangedInventoryItems)
	TArray<FName> ChangedInventoryItems;
	TArray<FName> Pending_ChangedInventoryItems;
	bool bIsWait_Pending_ChangedInventoryItems = false;
	UFUNCTION()
	void OnRep_ChangedInventoryItems();

	FTimerHandle AddTimerHandle;
	FTimerHandle RemoveTimerHandle;
	FTimerHandle ChangeTimerHandle;
};
