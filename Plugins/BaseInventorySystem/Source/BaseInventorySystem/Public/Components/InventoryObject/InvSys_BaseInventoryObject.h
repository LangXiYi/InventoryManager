// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "Components/InvSys_InventoryComponent.h"
#include "UObject/Object.h"
#include "InvSys_BaseInventoryObject.generated.h"

class UInvSys_InventoryComponent;


// FReplicationSystemFactory::CreateReplicationSystem(Params);	

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_BaseInventoryObject : public UObject
{
	GENERATED_BODY()

	friend class UInvSys_PreEditInventoryObject;
	
#define COPY_INVENTORY_OBJECT_PROPERTY(c, v)\
	v = static_cast<c*>(PreEditPayLoad)->v;

public:
	UInvSys_BaseInventoryObject();
	
	/** 初始化库存对象 */
	virtual void InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent, UObject* PreEditPayLoad);

	virtual void RefreshInventoryObject();

protected:
	virtual void CreateDisplayWidget(APlayerController* PC);
	
public:
	/**
	 * Getter Or Setter
	 **/

	FORCEINLINE FName GetSlotName() const;

	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const;

	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE ENetMode GetNetMode() const;

	FORCEINLINE bool IsLocallyControlled() const;

	FORCEINLINE AActor* GetOwner() const;

	FORCEINLINE float GetServerWaitBatchTime() const;

	FORCEINLINE virtual bool IsSupportedForNetworking() const override { return true; }

	/** 传入 Item Unique ID 判断其在库存对象中是否存在 */
	virtual bool ContainsItem(FName UniqueID);

	/** 从预设对象中复制属性 */
	virtual void CopyPropertyFromPreEdit(UInvSys_InventoryComponent* NewInventoryComponent,UObject* PreEditPayLoad);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	/** 在当前库存组件下具有唯一 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Object")
	FName SlotName;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Object")
	UInvSys_InventoryComponent* InventoryComponent = nullptr;

	bool bIsCopyPreEditData = false;

private:
	bool bIsInitInventoryObject = false;

	float ServerWaitBatchTime = 0.f;
};

/**
 * Pre Edit Inventory Object
 */

UCLASS(EditInlineNew, Blueprintable)
class BASEINVENTORYSYSTEM_API UInvSys_PreEditInventoryObject : public UObject
{
	GENERATED_BODY()
	
public:
	/** 在当前库存组件下具有唯一 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container Type")
	FName SlotName;
	
protected:
#define CONSTRUCT_INVENTORY_OBJECT(c) \
	virtual UInvSys_BaseInventoryObject* ConstructInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent)\
	{\
		check(NewInventoryComponent);\
		c* InvObj = NewObject<c>(NewInventoryComponent->GetOwner());\
		UE_LOG(LogInventorySystem, Log, TEXT("[%s] 正在构建库存对象: From [%s] ===> To [%s]。"),\
			NewInventoryComponent->HasAuthority() ? TEXT("Server") : TEXT("Client"),\
			*GetName(), *InvObj->GetName());\
		InvObj->CopyPropertyFromPreEdit(NewInventoryComponent, this);\
		return InvObj;\
	}

public:
	/** 容器构建函数 */
	CONSTRUCT_INVENTORY_OBJECT(UInvSys_BaseInventoryObject);
};
