// Fill out your copyright notice in the Description page of Project Settings.


#include "MPShooterAnimInstance.h"
#include "MPShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UMPShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MPShooterCharacter = Cast<AMPShooterCharacter>(TryGetPawnOwner());
}

void UMPShooterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MPShooterCharacter == nullptr)
	{
		MPShooterCharacter = Cast<AMPShooterCharacter>(TryGetPawnOwner());
	}
	if (MPShooterCharacter == nullptr) { return; }
	
	FVector Velocity = MPShooterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = MPShooterCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = MPShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = MPShooterCharacter->IsWeaponEquipped();
	bIsCrouched = MPShooterCharacter->bIsCrouched;
	bAiming = MPShooterCharacter->IsAiming();

	//Offset Yaw for Strafing
	FRotator AimRotation = MPShooterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MPShooterCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.0f);
	YawOffset = DeltaRotation.Yaw;
	
	/*if (!MPShooterCharacter->HasAuthority() && !MPShooterCharacter->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AimRotation Yaw %f:"), AimRotation.Yaw);
		UE_LOG(LogTemp, Warning, TEXT("MovementRotation Yaw %f:"), MovementRotation.Yaw);
	}*/
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MPShooterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.0f);
	Lean = FMath::Clamp(Interp, -90.0f, 90.0f);

}
