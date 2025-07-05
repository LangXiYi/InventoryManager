// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	UInvSys_BaseEquipContainerObject();

	virtual void RefreshInventoryObject(const FString& Reason = "") override;
	virtual void TryRefreshOccupant(const FString& Reason = "") override;
	virtual void TryRefreshContainerItems(const FString& Reason = "");
	
	/** [Server] 添加物品，并将操作记录至复制列表，等待一段时间后，将所有操作批量发送给客户端并清除操作列表。 */
	void RecordItemOperationByAdd(FName ItemUniqueID);
	/** [Server] 删除物品，并将操作记录至复制列表，等待一段时间后，将所有操作批量发送给客户端并清除操作列表。 */
	void RecordItemOperationByRemove(FName ItemUniqueID);
	/** [Server] 修改物品，并将操作记录至操作列表，等待一段时间后，将所有操作批量发送给客户端并清除操作列表。 */
	void RecordItemOperationByUpdate(FName ItemUniqueID);

protected:
	/** [Client & Server] 处理服务器批量返回的操作记录 */
	virtual void OnAddedContainerItems(const TArray<FName>& InAddedItems) {}
	/** [Client & Server] 处理服务器批量返回的操作记录 */
	virtual void OnRemovedContainerItems(const TArray<FName>& InRemovedItems) {}
	/** [Client & Server] 处理服务器批量返回的操作记录 */
	virtual void OnUpdatedContainerItems(const TArray<FName>& InChangedItems) {}

private:
	/** [Server] 将所有操作批量发送给客户端并清除操作列表 */
	void TryApplyAddOperations();
	/** [Server] 将所有操作批量发送给客户端并清除操作列表 */
	void TryApplyRemoveOperations();
	/** [Server] 将所有操作批量发送给客户端并清除操作列表 */
	void TryApplyUpdateOperations();

public:
	/**
	 * Getter Or Setter
	 **/

	virtual bool ContainsItem(FName UniqueID) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** [Server] 当前容器内的所有物品的 UniqueID */
	TSet<FName> ContainerItems;

	// TMap<FName, FInvSys_InventoryItem> ContainerItemInfos;
	
private:
	/*
	 * 注意：在同一时间内不得对同一物品进行不同操作，同一时间内对一个物品只能进行一种操作，该操作可以不限制次数，
	 * 如：在某一时刻对物品A记录了Add操作，但在发送操作前又进行另一操作
	 * 若网络出现波动，导致Add操作在另一操作之后出现，则在其他操作执行完成后，物品才姗姗来迟。
	 */
	
	UPROPERTY(ReplicatedUsing = OnRep_AddedInventoryItems)
	TArray<FName> AddedInventoryItems;
	TSet<FName> Pending_AddedInventoryItems;
	bool bIsWait_Pending_AddedInventoryItems = false;
	UFUNCTION()
	void OnRep_AddedInventoryItems();

	UPROPERTY(ReplicatedUsing = OnRep_RemovedInventoryItems)
	TArray<FName> RemovedInventoryItems;
	TSet<FName> Pending_RemovedInventoryItems;
	bool bIsWait_Pending_RemovedInventoryItems = false;
	UFUNCTION()
	void OnRep_RemovedInventoryItems();

	UPROPERTY(ReplicatedUsing = OnRep_ChangedInventoryItems)
	TArray<FName> ChangedInventoryItems;
	TSet<FName> Pending_ChangedInventoryItems;
	bool bIsWait_Pending_ChangedInventoryItems = false;
	UFUNCTION()
	void OnRep_ChangedInventoryItems();

	FTimerHandle AddTimerHandle;
	FTimerHandle RemoveTimerHandle;
	FTimerHandle ChangeTimerHandle;
};
