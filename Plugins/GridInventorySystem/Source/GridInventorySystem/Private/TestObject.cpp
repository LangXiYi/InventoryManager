// Fill out your copyright notice in the Description page of Project Settings.


#include "TestObject.h"

#include "Net/UnrealNetwork.h"

void UTestObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UTestObject, MyStruct, COND_None);
}
