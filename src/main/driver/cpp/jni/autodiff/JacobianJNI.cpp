// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include <jni.h>

#include <utility>
#include <vector>

#include <sleipnir/autodiff/jacobian.hpp>
#include <sleipnir/autodiff/variable.hpp>
#include <sleipnir/autodiff/variable_matrix.hpp>

#include "../SleipnirJNIUtil.hpp"
#include "org_wpilib_math_autodiff_JacobianJNI.h"
#include "wpi/jni_util.h"

using namespace wpi::java;

extern "C" {

/*
 * Class:     org_wpilib_math_autodiff_JacobianJNI
 * Method:    create
 * Signature: ([J[J)J
 */
JNIEXPORT jlong JNICALL
Java_org_wpilib_math_autodiff_JacobianJNI_create
  (JNIEnv* env, jclass, jlongArray variables, jlongArray wrt)
{
  JSpan<const jlong> variablesSpan{env, variables};
  slp::VariableMatrix variablesObj(slp::VariableMatrix::empty,
                                   variablesSpan.size(), 1);
  for (size_t i = 0; i < variablesSpan.size(); ++i) {
    variablesObj[i] =
        *reinterpret_cast<slp::Variable*>(variablesSpan[i]);
  }

  JSpan<const jlong> wrtSpan{env, wrt};
  slp::VariableMatrix wrtObj(slp::VariableMatrix::empty, wrtSpan.size(), 1);
  for (size_t i = 0; i < wrtSpan.size(); ++i) {
    wrtObj[i] = *reinterpret_cast<slp::Variable*>(wrtSpan[i]);
  }

  return reinterpret_cast<jlong>(
      new slp::Jacobian{std::move(variablesObj), std::move(wrtObj)});
}

/*
 * Class:     org_wpilib_math_autodiff_JacobianJNI
 * Method:    destroy
 * Signature: (J)V
 */
JNIEXPORT void JNICALL
Java_org_wpilib_math_autodiff_JacobianJNI_destroy
  (JNIEnv* env, jclass, jlong handle)
{
  delete reinterpret_cast<slp::Jacobian*>(handle);
}

/*
 * Class:     org_wpilib_math_autodiff_JacobianJNI
 * Method:    get
 * Signature: (J)[J
 */
JNIEXPORT jlongArray JNICALL
Java_org_wpilib_math_autodiff_JacobianJNI_get
  (JNIEnv* env, jclass, jlong handle)
{
  auto& jacobian = *reinterpret_cast<slp::Jacobian*>(handle);
  auto J = jacobian.get();

  std::vector<jlong> varHandles;
  varHandles.reserve(J.size());
  for (auto& var : J) {
    varHandles.emplace_back(
        reinterpret_cast<jlong>(new slp::Variable{var}));
  }

  return MakeJLongArray(env, varHandles);
}

/*
 * Class:     org_wpilib_math_autodiff_JacobianJNI
 * Method:    value
 * Signature: (J)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL
Java_org_wpilib_math_autodiff_JacobianJNI_value
  (JNIEnv* env, jclass, jlong handle)
{
  auto& jacobian = *reinterpret_cast<slp::Jacobian*>(handle);
  return wpi::math::detail::GetTriplets(env, jacobian.value());
}

}  // extern "C"
