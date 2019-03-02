/*
 * Copyright (C) 2018 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *--------------------------------------------------------------------------*/

/**
 * @ingroup groupExamples
 */

/**
 * @defgroup CNNExample Convolutional Neural Network Example
 *
 * \par Description:
 * \par
 * Demonstrates a convolutional neural network (CNN) example with the use of convolution,
 * ReLU activation, pooling and fully-connected functions.
 *
 * \par Model definition:
 * \par
 * The CNN used in this example is based on CIFAR-10 example from Caffe [1].
 * The neural network consists
 * of 3 convolution layers interspersed by ReLU activation and max pooling layers, followed by a
 * fully-connected layer at the end. The input to the network is a 32x32 pixel color image, which will
 * be classified into one of the 10 output classes.
 * This example model implementation needs 32.3 KB to store weights, 40 KB for activations and
 * 3.1 KB for storing the \c im2col data.
 *
 * \image html CIFAR10_CNN.gif "Neural Network model definition"
 *
 * \par Variables Description:
 * \par
 * \li \c conv1_wt, \c conv2_wt, \c conv3_wt are convolution layer weight matrices
 * \li \c conv1_bias, \c conv2_bias, \c conv3_bias are convolution layer bias arrays
 * \li \c ip1_wt, ip1_bias point to fully-connected layer weights and biases
 * \li \c input_data points to the input image data
 * \li \c output_data points to the classification output
 * \li \c col_buffer is a buffer to store the \c im2col output
 * \li \c scratch_buffer is used to store the activation data (intermediate layer outputs)
 *
 * \par CMSIS DSP Software Library Functions Used:
 * \par
 * - arm_convolve_HWC_q7_RGB()
 * - arm_convolve_HWC_q7_fast()
 * - arm_relu_q7()
 * - arm_maxpool_q7_HWC()
 * - arm_avepool_q7_HWC()
 * - arm_fully_connected_q7_opt()
 * - arm_fully_connected_q7()
 *
 * <b> Refer  </b>
 * \link arm_nnexamples_cifar10.cpp \endlink
 *
 * \par [1] https://github.com/BVLC/caffe
 */

#include "AUDIO_DISCO_F746NG.h"
#include "LCD_DISCO_F746NG.h"
#include "arm_nnexamples_cifar10_parameter.h"
#include "arm_nnexamples_cifar10_weights.h"

#include "arm_nnfunctions.h"
#include "arm_nnexamples_cifar10_inputs.h"

LCD_DISCO_F746NG lcd;
Serial pc(USBTX, USBRX);

// include the input and weights

static q7_t conv1_wt[CONV1_IM_CH * CONV1_KER_DIM * CONV1_KER_DIM * CONV1_OUT_CH] = CONV1_WT;
static q7_t conv1_bias[CONV1_OUT_CH] = CONV1_BIAS;

static q7_t conv2_wt[CONV2_IM_CH * CONV2_KER_DIM * CONV2_KER_DIM * CONV2_OUT_CH] = CONV2_WT;
static q7_t conv2_bias[CONV2_OUT_CH] = CONV2_BIAS;

static q7_t conv3_wt[CONV3_IM_CH * CONV3_KER_DIM * CONV3_KER_DIM * CONV3_OUT_CH] = CONV3_WT;
static q7_t conv3_bias[CONV3_OUT_CH] = CONV3_BIAS;

static q7_t ip1_wt[IP1_DIM * IP1_OUT] = IP1_WT;
static q7_t ip1_bias[IP1_OUT] = IP1_BIAS;

/* Here the image_data should be the raw uint8 type RGB image in [RGB, RGB, RGB ... RGB] format */
uint8_t   image_data[CONV1_IM_CH * CONV1_IM_DIM * CONV1_IM_DIM] = IMG_DATA;
uint8_t   image_data1[CONV1_IM_CH * CONV1_IM_DIM * CONV1_IM_DIM] = IMG_DATA1;

q7_t      output_data[IP1_OUT];
char      output_class[IP1_OUT][20] = {"airplane", "automobile", "bird", "cat", "deer", "dog", "frog", "horse", "ship", "truck"};

uint8_t  *test_image[2] = {image_data, image_data1};

//vector buffer: max(im2col buffer,average pool buffer, fully connected buffer)
q7_t      col_buffer[2 * 5 * 5 * 32 * 2];

q7_t      scratch_buffer[32 * 32 * 10 * 4];

int main()
{
  pc.baud(9600);
  printf("\rRunning CIFAR-10\n");
  printf("\r----------------------------\n");

  /* start the execution */
  for (int k = 0; k < 2; k++) {
	  
	  printf("\r Test Image : %d\n", k + 1);

	  q7_t     *img_buffer1 = scratch_buffer;
	  q7_t     *img_buffer2 = img_buffer1 + 32 * 32 * 32;

	  /* input pre-processing */
	  int mean_data[3] = INPUT_MEAN_SHIFT;
	  unsigned int scale_data[3] = INPUT_RIGHT_SHIFT;
	  for (int i=0;i<32*32*3; i+=3) {
	    img_buffer2[i] =   (q7_t)__SSAT( ((((int)test_image[k][i]   - mean_data[0])<<7) + (0x1<<(scale_data[0]-1)))
				     >> scale_data[0], 8);
	    img_buffer2[i+1] = (q7_t)__SSAT( ((((int)test_image[k][i+1] - mean_data[1])<<7) + (0x1<<(scale_data[1]-1)))
				     >> scale_data[1], 8);
	    img_buffer2[i+2] = (q7_t)__SSAT( ((((int)test_image[k][i+2] - mean_data[2])<<7) + (0x1<<(scale_data[2]-1)))
				     >> scale_data[2], 8);
	  }
	  
	  // conv1 img_buffer2 -> img_buffer1
	  arm_convolve_HWC_q7_RGB(img_buffer2, CONV1_IM_DIM, CONV1_IM_CH, conv1_wt, CONV1_OUT_CH, CONV1_KER_DIM, CONV1_PADDING,
				  CONV1_STRIDE, conv1_bias, CONV1_BIAS_LSHIFT, CONV1_OUT_RSHIFT, img_buffer1, CONV1_OUT_DIM,
				  (q15_t *) col_buffer, NULL);

	  arm_relu_q7(img_buffer1, CONV1_OUT_DIM * CONV1_OUT_DIM * CONV1_OUT_CH);

	  // pool1 img_buffer1 -> img_buffer2
	  arm_maxpool_q7_HWC(img_buffer1, CONV1_OUT_DIM, CONV1_OUT_CH, POOL1_KER_DIM,
			     POOL1_PADDING, POOL1_STRIDE, POOL1_OUT_DIM, NULL, img_buffer2);

	  // conv2 img_buffer2 -> img_buffer1
	  arm_convolve_HWC_q7_fast(img_buffer2, CONV2_IM_DIM, CONV2_IM_CH, conv2_wt, CONV2_OUT_CH, CONV2_KER_DIM,
				   CONV2_PADDING, CONV2_STRIDE, conv2_bias, CONV2_BIAS_LSHIFT, CONV2_OUT_RSHIFT, img_buffer1,
				   CONV2_OUT_DIM, (q15_t *) col_buffer, NULL);

	  arm_relu_q7(img_buffer1, CONV2_OUT_DIM * CONV2_OUT_DIM * CONV2_OUT_CH);

	  // pool2 img_buffer1 -> img_buffer2
	  arm_maxpool_q7_HWC(img_buffer1, CONV2_OUT_DIM, CONV2_OUT_CH, POOL2_KER_DIM,
			     POOL2_PADDING, POOL2_STRIDE, POOL2_OUT_DIM, col_buffer, img_buffer2);

	// conv3 img_buffer2 -> img_buffer1
	  arm_convolve_HWC_q7_fast(img_buffer2, CONV3_IM_DIM, CONV3_IM_CH, conv3_wt, CONV3_OUT_CH, CONV3_KER_DIM,
				   CONV3_PADDING, CONV3_STRIDE, conv3_bias, CONV3_BIAS_LSHIFT, CONV3_OUT_RSHIFT, img_buffer1,
				   CONV3_OUT_DIM, (q15_t *) col_buffer, NULL);

	  arm_relu_q7(img_buffer1, CONV3_OUT_DIM * CONV3_OUT_DIM * CONV3_OUT_CH);

	  // pool3 img_buffer-> img_buffer2
	  arm_maxpool_q7_HWC(img_buffer1, CONV3_OUT_DIM, CONV3_OUT_CH, POOL3_KER_DIM,
			     POOL3_PADDING, POOL3_STRIDE, POOL3_OUT_DIM, col_buffer, img_buffer2);

	  arm_fully_connected_q7_opt(img_buffer2, ip1_wt, IP1_DIM, IP1_OUT, IP1_BIAS_LSHIFT, IP1_OUT_RSHIFT, ip1_bias,
				     output_data, (q15_t *) img_buffer1);

	  arm_softmax_q7(output_data, 10, output_data);

	  /* print output */
	  for (int i = 0; i < IP1_OUT; i++) {
		  printf("\r%12s: %d %%\n", output_class[i], ((output_data[i] * 100)/128));
	  }

  }

  return 0;
}
