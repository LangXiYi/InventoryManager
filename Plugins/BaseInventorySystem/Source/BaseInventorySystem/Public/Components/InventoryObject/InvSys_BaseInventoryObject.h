// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "UObject/Object.h"
#include "InvSys_BaseInventoryObject.generated.h"

class UInvSys_EquipSlotWidget;
class UInvSys_InventoryComponent;


// FReplicationSystemFactory::CreateReplicationSystem(Params);	

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_BaseInventoryObject : public UObject
{
	GENERATED_BODY()

	friend UInvSys_InventoryComponent;
	friend class UInvSys_PreEditInventoryObject;
	
#define COPY_INVENTORY_OBJECT_PROPERTY(c, v)\
	v = static_cast<c*>(PreEditPayLoad)->v;

public:
	UInvSys_BaseInventoryObject();

	// [Server & Client] 在服务器创建库存对象后自动调用
	virtual void OnConstructInventoryObject(UInvSys_InventoryComponent* NewInvComp, UObject* PreEditPayLoad);
	
	/** 初始化库存对象，仅由客户端调用 */
	virtual void InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent, UObject* PreEditPayLoad);

	virtual void RefreshInventoryObject(const FString& Reason = "");

protected:
	virtual bool ReplicateSubobjects(UActorChannel *Channel, FOutBunch *Bunch, FReplicationFlags *RepFlags);

	/** 从预设对象中复制属性 */
	virtual void CopyPropertyFromPreEdit(UObject* PreEditPayLoad);
	
public:
	/**
	 * Getter Or Setter
	 **/

	/** 传入 Item Unique ID 判断其在库存对象中是否存在 */
	virtual bool ContainsItem(FName UniqueID);
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE FGameplayTag GetSlotTag() const{ return EquipSlotTag; }

	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const;

	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE ENetMode GetNetMode() const;

	FORCEINLINE AActor* GetOwner() const;

	FORCEINLINE bool IsReadyForReplication() const;

	FORCEINLINE bool IsUsingRegisteredSubObjectList();

	FORCEINLINE virtual bool IsSupportedForNetworking() const override { return true; }
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Object")
	UInvSys_InventoryComponent* InventoryComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Object")
	FGameplayTag EquipSlotTag;

private:
	bool bIsInitInventoryObject = false;
};

/**
 * Pre Edit Inventory Object
 */

UCLASS(EditInlineNew, Blueprintable)
class BASEINVENTORYSYSTEM_API UInvSys_PreEditInventoryObject : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container Type")
	FGameplayTag EquipSlotTag;
	
protected:
#define CONSTRUCT_INVENTORY_OBJECT(C) \
	virtual UInvSys_BaseInventoryObject* ConstructInventoryObject(UActorComponent* InvComp)\
	{\
		check(InvComp);\
		C* InvObj = NewObject<C>(InvComp->GetOwner());\
		return InvObj;\
	}

public:
	/** 容器构建函数 */
	CONSTRUCT_INVENTORY_OBJECT(UInvSys_BaseInventoryObject);
};
