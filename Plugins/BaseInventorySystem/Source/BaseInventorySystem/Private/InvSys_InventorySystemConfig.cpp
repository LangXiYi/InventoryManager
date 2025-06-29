// Fill out your copyright notice in the Description page of Project Settings.


#include "InvSys_InventorySystemConfig.h"

#if WITH_EDITOR

void UInvSys_InventorySystemConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	/*FName PropertyName = PropertyChangedEvent.GetPropertyName();
	FName MemberPropertyName = PropertyChangedEvent.GetMemberPropertyName();

	// 检查是否为 ConfigSettings 及其子属性（如：ConfigSettings.XX等）被修改。
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UIS_InventorySystemConfig, ConfigSettings) ||
		MemberPropertyName == GET_MEMBER_NAME_CHECKED(UIS_InventorySystemConfig, ConfigSettings))
	{
		Private_ConfigSettings = ConfigSettings;

		UE_LOG(LogInventorySystem, Log, TEXT("Inventory System Config Updated: %s"), *PropertyName.ToString());
	}*/
}

#endif