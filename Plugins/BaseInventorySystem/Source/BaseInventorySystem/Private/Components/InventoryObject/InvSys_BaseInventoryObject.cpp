// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"

#include "InvSys_InventorySystemConfig.h"
#include "Components/InvSys_InventoryComponent.h"

UInvSys_BaseInventoryObject::UInvSys_BaseInventoryObject()
{
	if (const UInvSys_InventorySystemConfig* InventorySystemConfig = GetDefault<UInvSys_InventorySystemConfig>())
	{
		ServerWaitBatchTime = InventorySystemConfig->ServerWaitBatchTime;
		// UE_LOG(LogInventorySystem, Log, TEXT("初始化ServerWaitBatchTime = %f"), ServerWaitBatchTime)
	}
}

void UInvSys_BaseInventoryObject::InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent,
                                                      UObject* PreEditPayLoad)
{
	check(NewInventoryComponent);
	if (bIsInitInventoryObject)
	{
		return;
	}
	bIsInitInventoryObject = true;
	// 复制预设数据至当前对象
	if (bIsCopyPreEditData == false)
	{
		CopyPropertyFromPreEdit(NewInventoryComponent, PreEditPayLoad);
	}
	// 创建显示效果
	/*if (NewInventoryComponent->GetPlayerController())
	{*/
		CreateDisplayWidget(NewInventoryComponent->GetPlayerController());
	/*}*/
}

void UInvSys_BaseInventoryObject::RefreshInventoryObject(const FString& Reason)
{
	if (Reason.IsEmpty())
	{
		return;
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("正在执行 RefreshInventoryObject() 操作。\n\tREASON: %s"), *Reason);
}

void UInvSys_BaseInventoryObject::CreateDisplayWidget(APlayerController* PC)
{
	UE_LOG(LogInventorySystem, Log, TEXT("[%s] 正在创建显示效果。"),
		PC->HasAuthority() ? TEXT("Server") : TEXT("Client"));
}

bool UInvSys_BaseInventoryObject::ContainsItem(FName UniqueID)
{
	return false;
}

void UInvSys_BaseInventoryObject::CopyPropertyFromPreEdit(UInvSys_InventoryComponent* NewInventoryComponent,
                                                          UObject* PreEditPayLoad)
{
	check(NewInventoryComponent);
	check(PreEditPayLoad);
	
	bIsCopyPreEditData = true;
	InventoryComponent = NewInventoryComponent;

	// todo:: 读取config file 文件?
	// ServerWaitBatchTime = 0.1f;


	UE_LOG(LogInventorySystem, Log, TEXT("[%s] 正在复制预设数据: From [%s] ===> To [%s]。"),
			HasAuthority() ? TEXT("Server") : TEXT("Client"),
			*PreEditPayLoad->GetName(), *this->GetName());

	COPY_INVENTORY_OBJECT_PROPERTY(UInvSys_PreEditInventoryObject, SlotName);

	// todo:: 复制完成后是否考虑移除该对象？移除后 DataAsset 中保存的对象是否也会被一起移除？
}

FName UInvSys_BaseInventoryObject::GetSlotName() const
{
	return SlotName;
}

UInvSys_InventoryComponent* UInvSys_BaseInventoryObject::GetInventoryComponent() const
{
	return InventoryComponent;
}

bool UInvSys_BaseInventoryObject::HasAuthority() const
{
	const AActor* Actor = GetOwner();
	check(Actor);
	return Actor->HasAuthority();
}

ENetMode UInvSys_BaseInventoryObject::GetNetMode() const
{
	const AActor* Actor = GetOwner();
	check(Actor);
	return Actor->GetNetMode();
}

bool UInvSys_BaseInventoryObject::IsLocallyControlled() const
{
	// HasLocalNetOwner 会循环到最顶层的Owner然后判断 IsLocallyControlled
	return GetOwner()->HasLocalNetOwner();
}

AActor* UInvSys_BaseInventoryObject::GetOwner() const
{
	return Cast<AActor>(GetOuter());
}

float UInvSys_BaseInventoryObject::GetServerWaitBatchTime() const
{
	return ServerWaitBatchTime;
}

void UInvSys_BaseInventoryObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
}
